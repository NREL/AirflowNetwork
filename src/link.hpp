#ifndef LINK_HPP
#define LINK_HPP

#include <string>
#include <functional>
#include "element.hpp"
#include "node.hpp"

namespace airflownetwork {

struct AirProperties;

template <typename I, typename P> struct Link
{
  Link(const std::string &name, Node<I,P> &node0, Node<I,P> &node1, Element<P> &element, double height0=0.0,
    double height1=0.0, double flow0=0.0, double flow1=0.0, double multiplier=1.0) : name(name), node0(node0), node1(node1),
    element(element), height0(height0), height1(height1), flow0(flow0), flow1(flow1), multiplier(multiplier), index0(0), index1(0)
  {}

  const std::string name;
  const Node<I, P>& node0;
  const Node<I, P>& node1;
  const Element<P>& element;
  
  double height0;
  double height1;

  double flow0;
  double flow1;
  double multiplier;

  I index0;
  I index1;
};

}

#endif // !LINK_HPP
