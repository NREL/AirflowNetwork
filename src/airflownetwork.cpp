#include <iostream>
#include "pugixml.hpp"
#include "model.hpp"

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "usage: airflownetwork <xml>" << std::endl;
    return 1;
  }
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(argv[1]);
  if (!result) {
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

  return 0;
}