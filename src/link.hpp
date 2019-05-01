#ifndef AIRFLOWNETWORK_LINK_HPP
#define AIRFLOWNETWORK_LINK_HPP

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
    element(element), height0(height0), height1(height1), stack_delta_p(0.0), added_delta_p(0.0), delta_p(0.0), flow(flow0-flow1), flow0(flow0), flow1(flow1), multiplier(multiplier), index0(0), index1(0)
  {}

  double upwind_stack_pressure() // This is maybe not a great name
  {
    double ps;
    if (flow > 0.0) {
      ps = 9.80 * (node0.density * (node0.height - node1.height) + height1 * (node0.density - node1.density));
    } else if (flow < 0.0) {
      ps = 9.80 * (node1.density * (node0.height - node1.height) + height0 * (node1.density - node0.density));
    } else {
      ps = 4.90 * ((node0.density + node1.density) * (node0.height - node1.height) + (height0 + height1) * (node1.density - node0.density));
    }
    return ps;
  }

  const std::string name;
  const Node<I, P>& node0;
  const Node<I, P>& node1;
  const Element<P>& element;
  
  double height0;
  double height1;

  double stack_delta_p;
  double added_delta_p;
  double delta_p;

  double flow;
  double flow0;
  double flow1;
  double multiplier;

  I index0;
  I index1;
};

}

#endif // !AIRFLOWNETWORK_LINK_HPP
