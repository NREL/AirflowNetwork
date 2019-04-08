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
  const char xmltext[]{ "<data><empty1></empty1><empty2/><text>This is the text</text><number>1</number><kids><kid/><kids><kid>One</kid></data>" };
  pugi::xml_document doc;
  auto root = doc.append_child("data");
  auto sub = root.append_child("empty1");
  sub = root.append_child("empty2");
  sub = root.append_child("text");
  auto text = sub.text();
  text.set("This is the text");
  sub = root.append_child("number");
  text = sub.text();
  text.set(1);
  
  doc.save_file("data.xml");

  std::cout << "Done" << std::endl;

  return 0;
}