#ifndef NODE_HPP
#define NODE_HPP

#include <string>
#include <unordered_map>
#include "node.hpp"

namespace airflownetwork {

enum class NodeType {Simulated, Fixed, Calculated};

template <typename I, typename P> struct Node
{
  Node(const std::string &name, double height=0.0, double temperature=293.15, double pressure=101325.0, double humidity_ratio=0.0)
    : name(name), height(height), temperature(temperature), pressure(pressure), humidity_ratio(humidity_ratio), 
    density(P::density(pressure, temperature, humidity_ratio)), sqrt_density(std::sqrt(density)), variable(false), index(0)
  {}

  const std::string name;
  double height;
  double temperature;
  double pressure;
  double humidity_ratio;
  double density;
  double sqrt_density;
  bool variable;
  I index;
};

}

#endif // !NODE_HPP
