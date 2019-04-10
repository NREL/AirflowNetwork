#ifndef MODEL_HPP
#define MODEL_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include "node.hpp"
#include "link.hpp"
#include "powerlaw.hpp"
#include "pugixml.hpp"
#include "skyline.hpp"

#define ORDERED_NODES

namespace airflownetwork {

template <typename I, typename P> struct Model
{
  Model(const std::string &name) : name(name)
  {

  }

  bool load(const pugi::xml_node &root)
  {
    bool success{ true };
    // Get the data from the XML
    auto elements = root.child("AirflowElements");
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

  bool linear_initialize()
  {
    skyline->clear();
    for (auto& link : links) {
      double C = link.element.linearize(link.node0, link.node1);
    }
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
#ifdef ORDERED_NODES
        // In the ordered case, only need to check for the possibility of a horrifying loop 
        if (i == j) {
          errors.push_back("Link \"" + el.name + "\" connects a node to itself and is a loop");
          return false;
        }
#else
        // Hijinks to avoid problems with unsigned types
        if (i < j) {
          i = el.node1.index;
          j = el.node0.index;
        } else if (i == j) {
          errors.push_back("Link \"" + el.name + "\" connects a node to itself and is a loop");
          return false;
        }
#endif
        h[j] = std::max(h[j], j - i);
      }
    }

    // Get the skyline solver set up
    skyline = std::make_unique<skyline::SymmetricMatrix<I, double, std::vector>>(h);

#ifdef ORDERED_NODES
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
#else

#endif

    return true;
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
      node = el.child("RelativeHeight");
      if (node) {
        height = node.text().as_double();
      }

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

    if (success) {
      // Build the overall lookup table, set up the pressure vector, and assign indices
      I i{ 0 };
      p.resize(simulated_nodes.size() + calculated_nodes.size() + fixed_nodes.size());
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

#ifdef ORDERED_NODES
      if (node0_ref.get().index < node1_ref.get().index) {
        links.emplace_back(name, node0_ref, node1_ref, element_ref, h[0], h[1], 0.0, 0.0, multiplier);
      } else {
        links.emplace_back(name, node1_ref, node0_ref, element_ref, h[1], h[0], 0.0, 0.0, multiplier);
      }
#else
      links.emplace_back(name, node0_ref, node1_ref, element_ref, h[0], h[1], 0.0, 0.0, multiplier);
#endif

    }
    return success;
  }

  bool load_state(const pugi::xml_node& state, std::string &label, double &P, double &T, double &W)
  {
    P = P::pressure_0;
    W = P::humidity_ratio_0;
    bool success{ true };
    auto subel = state.child("Temperature");
    if (subel) {
      T = subel.text().as_double();

      auto node = state.child("Pressure");
      if (node) {
        P = node.text().as_double();
      }

      node = state.child("HumidityRatio");
      if (node) {
        W = node.text().as_double();
      }

    } else {
      errors.push_back(label + " reference state does not have a specified temperature");
      success = false;
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

    double coefficient;
    auto node = element.child("Coefficient");
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

    node = element.child("ReferenceState");
    if (node) {
      double p0, T0, w0;
      if (load_state(node, "Power law element \"" + id +"\"", p0, T0, w0)) {
        powerlaw_elements.emplace_back(name, coefficient, exponent, p0, T0, w0);
      } else {
        // TODO: Handle failure here
      }
    } else {
      powerlaw_elements.emplace_back(id, coefficient, exponent);
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
    return success;
  }

public:
  std::string name;

  std::unordered_map<std::string, std::reference_wrapper<Node<I,P>>> node_lookup;
  //std::unordered_map<std::string, Node<I, P>&> node_lookup;
  std::vector<Node<I,P>> simulated_nodes;
  std::vector<Node<I,P>> fixed_nodes;
  std::vector<Node<I,P>> calculated_nodes;

  std::vector<Link<I,P>> links;

  std::unordered_map<std::string, std::reference_wrapper<AirflowElement<P>>> element_lookup;
  //std::unordered_map<std::string, AirflowElement<P>&> element_lookup;
  std::vector<PowerLaw<P>> powerlaw_elements;

  std::vector<std::string> errors;
  std::vector<std::string> warnings;

  std::vector<double> p;

  std::unique_ptr<skyline::SymmetricMatrix<I, double, std::vector>> skyline;

};

}

#endif // !MODEL_HPP

