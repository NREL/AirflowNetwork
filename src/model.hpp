// Copyright (c) 2019, Alliance for Sustainable Energy, LLC
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#ifndef MODEL_HPP
#define MODEL_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <fstream>
#include "node.hpp"
#include "material.hpp"
#include "link.hpp"
#include "powerlaw.hpp"
#include "pugixml.hpp"
#include "skyline.hpp"

namespace airflownetwork {

template <typename I, typename P> struct Model
{
  Model(const std::string &name) : name(name), tolerance(1.0e-4)
  {

  }

  bool load(const pugi::xml_node &root)
  {
    bool success{ true };
    // Get the data from the XML
    auto materials = root.child("Materials");
    if (materials) {
      success &= load_materials(materials);
    }
    auto elements = root.child("Elements");
    if (elements) {
      success &= load_elements(elements);
    }
    auto nodes = root.child("Nodes");
    if (nodes) {
      success &= load_nodes(nodes);
    }
    auto link_list = root.child("Links");
    if (link_list) {
      success &= load_links(link_list);
    }
    if (success) {
      success &= setup();
    }
    return success;
  }

  void linear_initialize()
  {
    // Clear out previous values
    skyline->fill(0.0);
    std::fill_n(p.begin(), simulated_nodes.size(), 0.0);

    // Fill in the coefficients
    for (auto& link : links) {
      double C = link.element.linearize(1.0, link.node0, link.node1);
      if (link.node0.variable) {
        // For node 0, the equation terms are C*(p0 - p1) = C*p0 - C*p1
        skyline->diagonal(link.node0.index) += C;
        if (link.node1.variable) {
          (*skyline)(link.index1) -= C;
          // For node 1, the equation terms are C*(p1 - p0) = C*p1 - C*p0
          skyline->diagonal(link.node1.index) += C;
        } else {
          // Move term to the RHS: b[node0] += C*p1
          p[link.node0.index] += C * p[link.node1.index];
        }
      } /* This path only needed for unordered nodes
        else {
        // Node 0 is not simulated, account for node 1
        // Node 1 had better be simulated, not checking here
        // For node 1, the equation terms are C*(p1 - p0) = C*p1 - C*p0
        skyline->diagonal(link.node1.index) += C;
        // Move term to the RHS: b[node0] += C*p1
        p[link.node1.index] += C * p[link.node0.index];
      }*/
    }

    for (auto& el : p) {
      std::cout << el << std::endl;
    }
    std::cout << std::endl;

    /*
    for (auto& el : skyline->diagonal()) {
      std::cout << el << std::endl;
    }
    std::cout << std::endl;
    */

    // Solve
    skyline->ldlt_solve(p);

    for (auto& el : p) {
      std::cout << el << std::endl;
    }

    // Copy the pressures into the nodes
    for(auto &node : simulated_nodes) {
      node.pressure = p[node.index];
    }

    // Compute flows
    std::array<double, 2> F, DF;
    for (auto& link : links) {
      double dp = link.node0.pressure - link.node1.pressure;
      link.element.calculate(false, dp, link.multiplier, 1.0, link.node0, link.node1, F, DF);
      link.flow = link.flow0 = F[0];
    }
  }

  void calculate_stack_pressures()
  {
    for (auto& link : links) {
      link.stack_delta_p = link.upwind_stack_pressure();
    }
  }

  void steady_solve()
  {
    calculate_stack_pressures();

    double alpha = 1.0;

    double delta_max = 1.0e6;
    int iter_max{ 25 };
    int iter{ 1 };
    double sum_max{ 0.0 };
    // Will always do at least one iteration, so split out the first iteration
    for (auto& link : links) {
      link.delta_p = link.node0.pressure - link.node1.pressure + link.stack_delta_p + link.added_delta_p;
    }
    // Fill the Jacobian matrix
    filjac();
    // Solve the system
    skyline->ldlt_solve(sum);
    // Update the pressures in the nodes
    for (auto& node : simulated_nodes) {
      node.pressure -= alpha * sum[node.index];
    }
    // At this point, the pressures are updated for one iteration, but the flows are not

    do {
      // Compute the pressure differences across the links
      for (auto& link : links) {
        link.delta_p = link.node0.pressure - link.node1.pressure + link.stack_delta_p + link.added_delta_p;
      }

      // Fill the Jacobian matrix, which updates the flows as well
      filjac();

      // Check for convergence here
      for (size_t i = 0; i < simulated_nodes.size(); ++i) {
        sum_max = std::max(sum_max, std::abs(sum[i]));
      }

      std::cout << iter << ' ' << sum_max << std::endl;

      if (sum_max < tolerance) {
        //break;
        return; // Need to return convergence information
      }

      // Solve the system
      skyline->ldlt_solve(sum);
      ++iter;
      // Update the pressures in the nodes
      for (auto& node : simulated_nodes) {
        node.pressure -= alpha * sum[node.index];
      }

      // Update
      delta_max = 0.0;
      for (size_t i = 0; i < simulated_nodes.size(); ++i) {
        delta_max = std::max(delta_max, std::abs(sum[i]));
        p[i] -= alpha*sum[i];
      }
     
      // Copy the pressures into the nodes
      for (auto& node : simulated_nodes) {
        node.pressure = p[node.index];
      }

    } while (iter < iter_max); // delta_max > tolerance);

    // Only get to here if there's a convergence failure
    std::cout << "Convergence Failure" << std::endl;
    
  }

  bool validate_network()
  {
    for (auto& link : links) {
      if (!(link.node0.variable || link.node1.variable)) {
        errors.push_back("Link \"" + link.name + "\" connects two non-simulated nodes");
        return false;
      }
    }
    return true;
  }

  bool save(const std::string& filename) const
  {
    pugi::xml_document doc;
    auto root = doc.append_child("AirflowNetwork");
    root.append_attribute("xmlns") = "http://github.com/jasondegraw/AirflowNetwork";
    root.append_attribute("xmlns:xsi") = "http://www.w3.org/2001/XMLSchema-instance";
    //root.append_attribute("xmlns:xsd") = "http://www.w3.org/2001/XMLSchema";
    root.append_attribute("xsi:schemaLocation") = "http://github.com/jasondegraw/AirflowNetwork airflownetwork.xsd";

    auto xml_links = root.append_child("Links");
    for (auto& link : links) {
      auto el = xml_links.append_child("Link");
      el.append_attribute("ID") = link.name.c_str();
    }

    auto xml_flow_results = root.append_child("FlowResults");
    auto xml_flow_result = xml_flow_results.append_child("FlowResult");
    for (auto& link : links) {
    }

    std::ofstream file(filename);
    if (file.is_open()) {
      doc.save(file, "  ");
      file.close();
      return true;
    }
    return false;
  }

  bool open_output(const std::string& basepath)
  {
    m_pressure_output = std::make_unique<std::ofstream>(basepath + "_p.csv");
    m_flow_output = std::make_unique<std::ofstream>(basepath + "_f.csv");
    if(!(m_pressure_output->is_open() && m_flow_output->is_open())) {
      return false;
    }
    *m_pressure_output << "Time(s)";
    for (auto& node : simulated_nodes) {
      *m_pressure_output << ',' << node.name;
    }
    *m_pressure_output << std::endl;

    *m_flow_output << "Time(s)";
    for (auto& link : links) {
      *m_flow_output << ',' << link.name;
    }
    *m_flow_output << std::endl;
    return true;
  }

  bool write_output(double seconds)
  {
    *m_pressure_output << seconds;
    for (auto& node : simulated_nodes) {
      *m_pressure_output << ',' << node.pressure;
    }
    *m_pressure_output << std::endl;

    *m_flow_output << seconds;
    for (auto& link : links) {
      *m_flow_output << ',' << link.flow;
    }
    *m_flow_output << std::endl;
    return true;
  }

  bool close_output()
  {
    m_pressure_output->close();
    m_pressure_output.reset();
    m_flow_output->close();
    m_flow_output.reset();
    return true;
  }

  bool explicit_transport(I cxi, std::vector<double>& CN, std::vector<double>& C0)
  {

  }

private:
  bool setup()
  {
    // Figure out the skyline heights
    std::vector<I> h(simulated_nodes.size(), 0);
    for (auto& el : links) {
      if (el.node0.variable && el.node1.variable) {
        I i = el.node0.index;
        I j = el.node1.index;
#ifdef UNORDERED_NODES
        // Hijinks to avoid problems with unsigned types
        if (i < j) {
          i = el.node1.index;
          j = el.node0.index;
        } else if (i == j) {
          errors.push_back("Link \"" + el.name + "\" connects a node to itself and is a loop");
          return false;
        }
#else
        // In the ordered case, only need to check for the possibility of a horrifying loop 
        if (i == j) {
          errors.push_back("Link \"" + el.name + "\" connects a node to itself and is a loop");
          return false;
        }
#endif
        h[j] = std::max(h[j], j - i);
      }
    }

    // Get the skyline solver set up
    skyline = std::make_unique<skyline::SymmetricMatrix<I, double, std::vector>>(h);

#ifdef UNORDERED_NODES
    
#else
    for (auto& el : links) {
      I i = el.node0.index;
      I j = el.node1.index;

      // Only get the index for the 1 side of the link, and only if that node is simulatd.
      // This is legit because the matrix is symmetric and we're only storing the upper
      // triangular part.
      if (el.node1.variable) {
        auto index = skyline->index(i, j);
        if (!index) {
          errors.push_back("Link \"" + el.name + "\", node \"" + el.node1.name + "\" has an index outside the skyline");
          return false;
        }
        el.index1 = index.value();
      }
  }
#endif

    return true;
  }

  void filjac()
  {
    skyline->fill(0.0);
    std::fill(sum.begin(), sum.end(), 0.0);
    std::array<double, 2> F;
    std::array<double, 2> DF;
    // Loop over the links and build the Jacobian
    size_t i = 0;
    for (auto& link : links) {
      if (link.node0.variable) {
        int nf = link.element.calculate(false, link.delta_p, link.multiplier, link.control, link.node0, link.node1, F, DF);
        if (nf == 1) {
          skyline->diagonal(link.node0.index) += DF[0];
          sum[link.node0.index] += F[0];
          if (link.node1.variable) {
            (*skyline)(link.index1) -= DF[0];
            skyline->diagonal(link.node1.index) += DF[0];
            sum[link.node1.index] -= F[0];
          }
          link.flow = link.flow0 = F[0];
        } else {
          // Later
        }
      }
      ++i;
    }

  }

  bool load_materials(const pugi::xml_node& xml_materials)
  {
    bool success{ true };
    int material_count{ 0 };
    for (pugi::xml_node el : xml_materials.children("Material")) {
      ++material_count;
      std::string name;
      auto attr = el.attribute("ID");
      if (attr) {
        name = attr.as_string();
      } else {
        errors.push_back("Material #" + std::to_string(material_count) + " does not have an ID");
        success = false;
        continue;
      }

      double default_conc;
      auto node = el.child("DefaultConcentration");
      if (node) {
        default_conc = node.text().as_double();
      } else {
        errors.push_back("Material \"" + name + "\" does not have a default concentration");
        success = false;
        continue;
      }

      materials.emplace_back(name, default_conc);

    }

    return success;
  }

  bool load_nodes(const pugi::xml_node &nodes)
  {
    bool success{ true };
    int node_count{0};
    for (pugi::xml_node el : nodes.children("Node")) {
      ++node_count;
      std::string name;
      auto attr = el.attribute("ID");
      if (attr) {
        name = attr.as_string();
      } else {
        errors.push_back("Node #" + std::to_string(node_count) + " does not have an ID");
        success = false;
        continue;
      }
      NodeType type{ NodeType::Simulated };
      auto node = el.child("PressureHandling");
      if (node) {
        std::string text = node.text().as_string();
        if (text == "Fixed") {
          type = NodeType::Fixed;
        } else if (text == "Calculated") {
          type = NodeType::Calculated;
        } else if (text != "Simulated") {
          warnings.push_back("Node \"" + name + "\" has unrecognized pressure handling specification \"" + text + "\", defaulting to \"Simulated\"");
        }
      }

      double height = 0.0;

      std::string level_id;
      node = el.child("LevelID");
      if (node) {
        auto attr = node.attribute("IDref");
      }
      
      node = el.child("RelativeHeight");
      if (node) {
        height += node.text().as_double();
      }

      node = el.child("DefaultState");
      if (node) {
        double P, T, W;
        success &= load_state(node, "Node \"" + name + "\"", P, T, W);

        switch (type) {
        case NodeType::Simulated:
          simulated_nodes.emplace_back(name, height, P, T, W);
          break;
        case NodeType::Fixed:
          fixed_nodes.emplace_back(name, height, P, T, W);
          break;
        case NodeType::Calculated:
          calculated_nodes.emplace_back(name, height, P, T, W);
          break;
        }
      } else {
        switch (type) {
        case NodeType::Simulated:
          simulated_nodes.emplace_back(name, height);
          break;
        case NodeType::Fixed:
          fixed_nodes.emplace_back(name, height);
          break;
        case NodeType::Calculated:
          calculated_nodes.emplace_back(name, height);
          break;
        }
      }
    }

    if (success) {
      // Build the overall lookup table, set up the pressure vector, and assign indices
      I i{ 0 };
      p.resize(simulated_nodes.size() + calculated_nodes.size() + fixed_nodes.size());
      sum.resize(simulated_nodes.size());
      for (auto& el : simulated_nodes) {
        node_lookup.emplace(el.name, el);
        el.variable = true;
        el.index = i;
        ++i;
      }
      for (auto& el : fixed_nodes) {
        node_lookup.emplace(el.name, el);
        el.index = i;
        p[i] = el.pressure;
        ++i;
      }
      for (auto& el : calculated_nodes) {
        node_lookup.emplace(el.name, el);
        el.index = i;
        p[i] = el.pressure;
        ++i;
      }
    }

    return success;
  }

  bool load_links(const pugi::xml_node& link_list)
  {
    bool success{ true };
    int link_count{ 0 };
    for (pugi::xml_node el : link_list.children("Link")) {
      ++link_count;
      std::string name;
      auto attr = el.attribute("ID");
      if (attr) {
        name = attr.as_string();
      } else {
        errors.push_back("Link #" + std::to_string(link_count) + " does not have an ID");
        success = false;
        continue;
      }

      std::string element_id;
      auto node = el.child("ElementID");
      if (node) {
        auto attr = node.attribute("IDref");
        if (attr) {
          element_id = attr.as_string();
        } else {
          errors.push_back("Link #" + std::to_string(link_count) + " does not have an element IDref");
          success = false;
          continue;
        }
      } else {
        errors.push_back("Link #" + std::to_string(link_count) + " does not have an element specified");
        success = false;
        continue;
      }

      double multiplier{ 1.0 };
      node = el.child("Multiplier");
      if (node) {
        multiplier = node.text().as_double();
      }

      std::array<double, 2> h{ { 0.0, 0.0 } };
      std::array<std::string, 2> n_name;
      node = el.child("Nodes");
      if (node) {
        int count = 0;
        for (auto& el : node.children("Node")) {
          if (count >= 2) {
            errors.push_back("Link #" + std::to_string(link_count) + " has more the two associated nodes");
            success = false;
            continue;
          }
          auto subel = el.child("NodeID");
          if (subel) {
            auto attr = subel.attribute("IDref");
            if (attr) {
              n_name[count] = attr.as_string();
            } else {
              errors.push_back("Link #" + std::to_string(link_count) + " does not have an element IDref");
              success = false;
              continue;
            }
          } else {
            errors.push_back("Link #" + std::to_string(link_count) + ", node # " + std::to_string(count + 1) + " does not have a node ID");
            success = false;
            continue;
          }

          subel = el.child("RelativeHeight");
          if (subel) {
            h[count] = subel.text().as_double();
          }

          ++count;
        }

        if (count < 2) {
          errors.push_back("Link #" + std::to_string(link_count) + " has fewer than two associated nodes");
          success = false;
          continue;
        }

      } else {
        errors.push_back("Link #" + std::to_string(link_count) + " does not have any associated nodes");
        success = false;
        continue;
      }
      // Find the element
      auto found_element = element_lookup.find(element_id);
      if (found_element == element_lookup.end()) {
        errors.push_back("Element \"" + element_id + "\" specified in link \"" + name + "\" does not exist");
        success = false;
        continue;
      }
      auto element_ref{ found_element->second };

      // Find the nodes
      std::cout << n_name[0] << ' ' << n_name[1] << std::endl;
      auto found_node = node_lookup.find(n_name[0]);
      if (found_node == node_lookup.end()) {
        errors.push_back("Node \"" + n_name[0] + "\" specified in link \"" + name + "\" does not exist");
        success = false;
        continue;
      }
      auto node0_ref{ found_node->second };

      found_node = node_lookup.find(n_name[1]);
      if (found_node == node_lookup.end()) {
        errors.push_back("Node \"" + n_name[1] + "\" specified in link \"" + name + "\" does not exist");
        success = false;
        continue;
      }
      auto node1_ref{ found_node->second };

#ifdef UNORDERED_NODES
      links.emplace_back(name, node0_ref, node1_ref, element_ref, h[0], h[1], 0.0, 0.0, multiplier);
#else
      if (node0_ref.get().index < node1_ref.get().index) {
        links.emplace_back(name, node0_ref, node1_ref, element_ref, h[0], h[1], 0.0, 0.0, multiplier);
      } else {
        links.emplace_back(name, node1_ref, node0_ref, element_ref, h[1], h[0], 0.0, 0.0, multiplier);
      }
#endif

    }
    return success;
  }

  bool load_state(const pugi::xml_node& state, std::string &label, double &P, double &T, double &W)
  {
    T = P::temperature_0;
    P = P::pressure_0;
    W = P::humidity_ratio_0;
    bool success{ true };
    auto node = state.child("Temperature");
    if (node) {
      T = node.text().as_double();
      auto attr = node.attribute("units");
      if (!attr) {
        errors.push_back(label + " temperature is missing the required units attribute");
        return false;
      }

      std::string units_string = attr.as_string();
      if (units_string == "K") {
        T -= 273.15;
      } else if (units_string == "F") {
        T = 5.0 * (T - 32.0) / 9.0;
      } else if (units_string == "R") {
        T = 5.0 * (T - 491.67) / 9.0;
      } else if (units_string != "C") {
        errors.push_back(label + " temperature unit \"" + units_string + "\" is not recognized");
        return false;
      }
    }

    node = state.child("Pressure");
    if (node) {
      P = node.text().as_double();
      auto attr = node.attribute("units");
      if (!attr) {
        errors.push_back(label + " pressure is missing the required units attribute");
        return false;
      }
      
      // Need more pressure units here
      std::string units_string = attr.as_string();
      if (units_string == "Pag") {
        P += 101325.0;
      } else if (units_string != "Pa") {
        errors.push_back(label + " pressure unit \"" + units_string + "\" is not recognized");
        return false;
      }
      
    }

    node = state.child("HumidityRatio");
    if (node) {
      W = node.text().as_double();
    }

    return success;
  }

  bool load_powerlaw(const pugi::xml_node& element, int count)
  {
    std::string id;
    auto attr = element.attribute("ID");
    if (attr) {
      id = attr.as_string();
    } else {
      errors.push_back("Power law element #" + std::to_string(count) + " does not have an element ID");
      return false;
    }

    bool contam{ false };
    auto node = element.child("Formulation");
    if (node) {
      std::string value = node.text().as_string();
      if (value == "CONTAM") {
        contam = true;
      } else if (value != "AIRNET") {
        errors.push_back("Element \"" + id + "\" has unsupported formulation \"" + value + "\"");
        return false;
      }
    }

    double coefficient;
    node = element.child("Coefficient");
    if (node) {
      coefficient = node.text().as_double();
    } else {
      errors.push_back("Element \"" + id + "\" does not specify a coefficient");
      return false;
    }

    double laminar_coefficient = coefficient;
    node = element.child("LaminarCoefficient");
    if (node) {
      laminar_coefficient = node.text().as_double();
    }

    double exponent{ 0.65 };
    node = element.child("Exponent");
    if (node) {
      exponent = node.text().as_double();
    }

    // Stop here for now, only support AIRNET/E+ max flow selection
    if (contam) {
      contamx_powerlaw_elements.emplace_back(id, coefficient, laminar_coefficient, exponent);
    } else {
      node = element.child("ReferenceState");
      if (node) {
        double p0, T0, w0;
        if (load_state(node, "Power law element \"" + id + "\"", p0, T0, w0)) {
          powerlaw_elements.emplace_back(id, coefficient, laminar_coefficient, exponent, p0, T0, w0);
        } else {
          // TODO: Handle failure here
          errors.push_back("Failed to load power law element \"" + id + "\"");
        }
      } else {
        powerlaw_elements.emplace_back(id, coefficient, laminar_coefficient, exponent);
      }
    }
    
    return true;
  }

  bool load_elements(const pugi::xml_node& elements)
  {
    bool success{ true };
    int count = 1;
    for (auto& node : elements.children("PowerLaw")) {
      success |= load_powerlaw(node, count);
      ++count;
    }
    if (!success) {
      return false;
    }
    // Load other elements here

    // Build the lookup table
    for (auto& el : powerlaw_elements) {
      // Check for clashes?
      element_lookup.emplace(el.name, el);
    }

    for (auto& el : contamx_powerlaw_elements) {
      // Check for clashes?
      element_lookup.emplace(el.name, el);
    }
    return success;
  }

public:
  std::string name;

  std::unordered_map<std::string, std::reference_wrapper<Node<I,P>>> node_lookup;
  //std::unordered_map<std::string, Node<I, P>&> node_lookup;
  std::vector<Node<I,P>> simulated_nodes;
  std::vector<Node<I,P>> fixed_nodes;
  std::vector<Node<I,P>> calculated_nodes;

  std::vector<Material> materials;

  std::vector<Link<I,P>> links;

  std::unordered_map<std::string, std::reference_wrapper<Element<P>>> element_lookup;
  //std::unordered_map<std::string, Element<P>&> element_lookup;
  std::vector<PowerLaw<P>> powerlaw_elements;
  std::vector<ContamXPowerLaw<P>> contamx_powerlaw_elements;

  std::vector<std::string> errors;
  std::vector<std::string> warnings;

  std::vector<double> p; // Current pressure value, sized to match the total number of nodes
  std::vector<double> sum; // RHS for solution, sized to match the number of simulated nodes

  std::unique_ptr<skyline::SymmetricMatrix<I, double, std::vector>> skyline;
  double tolerance;

private:
  std::unique_ptr<std::ofstream> m_pressure_output;
  std::unique_ptr<std::ofstream> m_flow_output;

};

}

#endif // !MODEL_HPP

