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
#include <iostream>
#include "pugixml.hpp"
#include "model.hpp"
#include "results.hpp"

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "usage: airflownetwork <xml>" << std::endl;
    return 1;
  }
  pugi::xml_document doc;
  pugi::xml_parse_result parse_result = doc.load_file(argv[1]);
  if (!parse_result) {
    std::cerr << "Failed to load XML file name \"" << argv[1] << '\"' << std::endl;
  }

  std::string root_name{ "AirflowNetwork" };
  auto afn = doc.child(root_name.c_str());
  if (!afn) {
    std::cerr << "Failed to find root AirflowNetwork node" << std::endl;
    return 1;
  }

  airflownetwork::Model<size_t, airflownetwork::properties::AIRNET> model("main");
  if (!model.load(afn)) {
    std::cerr << "Failed to load AirflowNetwork model" << std::endl;
    for (auto& mesg : model.errors) {
      std::cerr << mesg << std::endl;
    }
    return 1;
  }

  int element_count = 0;
  std::cout << "Elements ------------- " << std::endl;
  for (auto& el : model.powerlaw_elements) {
    ++element_count;
    std::cout << "\tPower Law:" << el.name << std::endl;
  }
  std::cout << "Found " << element_count << " Element(s)" << std::endl;

  int material_count = 0;
  std::cout << "Materials ------------ " << std::endl;
  for (auto& el : model.materials) {
    ++material_count;
    std::cout << "\tMaterial:" << el.name << std::endl;
  }
  std::cout << "Found " << material_count << " Material(s)" << std::endl;

  int node_count = 0;
  std::cout << "Nodes ---------------- " << std::endl;
  for (auto& el : model.simulated_nodes) {
    std::cout << "\tSimulated:" << el.name << " [" << el.index << ']' << std::endl;
    ++node_count;
  }
  for (auto& el : model.fixed_nodes) {
    std::cout << "\tFixed:" << el.name << " [" << el.index << ']' << std::endl;
    ++node_count;
  }
  for (auto& el : model.calculated_nodes) {
    std::cout << "\tCalculated:" << el.name << " [" << el.index << ']' << std::endl;
    ++node_count;
  }
  /*
  auto nodes = afn.child("Nodes");
  if (nodes) {
    for (pugi::xml_node el : nodes.children("Node")) {
      ++node_count;
      std::string name;
      auto attr = el.attribute("ID");
      if (attr) {
        name = attr.as_string();
      } else {
        // Error
      }
      std::cout << '\t' << name << '(' << name.length() << ')' << std::endl;
      //for (auto i = 0; i < name.length(); i++) {
      //  std::cout << std::hex << (short)name[i] << std::endl;
      //}
    }
  }
  */

  std::cout << "Found " << node_count << " Node(s)" << std::endl;

  int link_count = 0;
  std::cout << "Links ---------------- " << std::endl;
  for (auto& el : model.links) {
    ++link_count;
    std::cout << '\t' << el.name << '(' << el.node0.name << "--" << el.element.name << "-->" << el.node1.name << "), ["
      << el.index0 << ',' << el.index1 << ']' << std::endl;
  }

  std::cout << "Found " << link_count << " Link(s)" << std::endl;

  if (!model.validate_network()) {
    for (auto& mesg : model.errors) {
      std::cerr << mesg << std::endl;
    }
    return 1;
  }

  // Read in flow results
  airflownetwork::Results<airflownetwork::Link<size_t, airflownetwork::properties::AIRNET>> results;
  results.load(afn, model.links);

  for (auto& mesg : results.errors) {
    std::cerr << mesg << std::endl;
  }

  if (results.link_flows.size() > 0) {
    std::cout << "Flows ---------------- " << std::endl;
    int count{ 0 };
    for (auto& el : results.link_flows[0].results) {
      ++count;
      std::cout << '\t' << count << ' ' << el.object.name << ' ' << el.flow << std::endl;
    }

    results.link_flows[0].apply();
  }

  //model.linear_initialize();

  //model.save("out.xml");

  //model.open_output("output");
  //model.write_output(0.0);

  //model.steady_solve();
  //model.write_output(0.0);

  //model.close_output();

  return 0;
}