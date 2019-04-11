#ifndef NODE_HPP
#define NODE_HPP

#include <string>
#include "properties.hpp"

namespace airflownetwork {

enum class NodeType {Simulated, Fixed, Calculated};

template <typename I, typename P> struct Node : State<P>
{
  Node(const std::string &name, double height=0.0, double pressure=P::pressure_0, double temperature = P::temperature_0,
    double humidity_ratio=P::humidity_ratio_0) : State<P>(pressure, temperature, humidity_ratio), name(name), height(height),
    variable(false), index(0)
  {}

  const std::string name;
  double height;
  bool variable;
  I index;
};

}

#endif // !NODE_HPP
