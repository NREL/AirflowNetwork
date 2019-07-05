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
    element(element), height0(height0), height1(height1), stack_delta_p(0.0), added_delta_p(0.0), delta_p(0.0), flow(flow0-flow1), flow0(flow0), flow1(flow1), multiplier(multiplier), control(1.0), index0(0), index1(0)
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

  void set_flow(double f)
  {
    flow = flow0 = f;
    flow1 = 0.0;
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
  double control;

  I index0;
  I index1;
};

}

#endif // !AIRFLOWNETWORK_LINK_HPP
