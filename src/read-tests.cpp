#include <iostream>
#include "pugixml.hpp"

void evaluate_node(const pugi::xml_node &node, const char *name)
{
  if (node) {
    if (node.empty()) { // This is redundant, but check it anyway
      std::cout << name << " node is empty!" << std::endl;
    } else {
      std::cout << name << " value: '" << node.value() << "'" << std::endl;
      if (node.text()) {
        std::cout << ' ' << name << " text: '" << node.text() << "'" << std::endl;
        std::cout << name << " text content: '" << node.text().as_string() << "'" << std::endl;
      } else {
        std::cout << name << " text is empty ('" << node.text().as_string() << "')" << std::endl;
      }
      if (node.first_child()) {
        int i{0};
        for (auto &kid : node.children()) {
          std::cout << "\tchild " << ++i << ": " << kid.text().as_string() << std::endl;
        }
      } else {
        std::cout << name << " has no children" << std::endl;
      }
    }
  } else {
    std::cout << "Failed to find " << name << " node" << std::endl;
  }
}

int main()
{
  const char xmltext[]{"<data><empty1></empty1><empty2/><text>This is the text</text><number>1</number><kids><kid/><kids><kid>One</kid></data>"};
  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_string(xmltext);
  if (!result) {
    std::cerr << "Failed to load XML string" << std::endl;
  }


  std::string root_name{ "data" };
  auto data = doc.child(root_name.c_str());
  if (!data) {
    std::cerr << "Failed to find data node" << std::endl;
    return 1;
  }

  auto empty1 = data.child("empty1");
  evaluate_node(empty1, "empty1");
  std::cout << std::endl;

  auto empty2 = data.child("empty2");
  evaluate_node(empty2, "empty2");
  std::cout << std::endl;

  auto empty3 = data.child("empty3");
  evaluate_node(empty3, "empty3");
  std::cout << std::endl;

  auto number = data.child("number");
  evaluate_node(number, "number");
  std::cout << std::endl;

  auto text = data.child("text");
  evaluate_node(text, "text");
  std::cout << std::endl;

  auto kids = data.child("kids");
  evaluate_node(kids, "kids");
  std::cout << std::endl;

  /*
  int zone_count = 0;
  std::cout << "Zones ------ " << std::endl;
  for (pugi::xml_node zone : gbxml.children("Zone")) {
    ++zone_count;
    std::string name = zone.child("Name").text().as_string();
    std::cout << '\t' << name << '(' << name.length() << ')' << std::endl;
    for (auto i = 0; i < name.length(); i++) {
      std::cout << std::hex << (short)name[i] << std::endl;
    }
  }

  std::cout << "Found " << zone_count << " zone(s)" << std::endl;

  auto campus = gbxml.child("Campus");
  if (!campus) {
    std::cerr << "Failed to find Campus node" << std::endl;
    return 1;
  }

  int building_count{0};
  int space_count{0};
  std::cout << "Buildings -- " << std::endl;
  for (auto &building : campus.children("Building")) {
    ++building_count;
    // Get the name and type of the building
    std::string name = building.child("Name").text().as_string();
    if (name.empty()) {
      name = '~';
    }
    auto type_attr = building.attribute("buildingType");
    if (!type_attr) {
      name += " (no type)";
    } else {
      name += " (" + std::string(type_attr.as_string()) + ')';
    }
    std::cout << name << std::endl;
    std::cout << "   Spaces -- " << std::endl;
    for (auto &space : building.children("Space")) {
      ++space_count;
      std::string name = space.child("Name").text().as_string();
      std::cout << '\t' << name << std::endl;
    }
  }

  std::cout << "Found " << building_count << " building(s) with " << space_count << " space(s)" << std::endl;

  doc.save_file("test.xml");

  */

  std::cout << "Done" << std::endl;

  return 0;
}