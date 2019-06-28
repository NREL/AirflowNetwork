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
#ifndef AIRFLOWNETWORK_RESULTS_HPP
#define AIRFLOWNETWORK_RESULTS_HPP

#include <string>
#include <vector>
#include "properties.hpp"

namespace airflownetwork {

template <typename L> struct Flow
{
  Flow(L& object, double flow) : object(object), flow(flow)
  {}

  L& object;
  const double flow;
};

template <typename L> struct FlowResult
{

  FlowResult(int time, std::vector<Flow<L>>& results) : time(time), results(results)
  {}

  void apply()
  {
    for (auto& el : results) {
      el.object.set_flow(el.flow);
    }
  }

  int time; // Time in seconds right now
  std::vector<Flow<L>> results;
};

template <typename L> struct Results
{
  bool load(const pugi::xml_node& root, std::vector<L>& links)
  {
    bool success{ true };
    // Get the data from the XML
    auto flows = root.child("FlowResults");
    if (flows) {
      success &= load_flow_results(flows, links);
    }

    return success;
  }

  std::vector<FlowResult<L>> link_flows;

  std::vector<std::string> errors;
  std::vector<std::string> warnings;

private:
  bool load_flow_results(const pugi::xml_node& flows, std::vector<L>& links)
  {
    bool success{ true };
    int count{ 0 };
    for (pugi::xml_node el : flows.children("FlowResult")) {
      ++count;
      // Should these have names? Maybe
      //std::string name;
      //auto attr = el.attribute("ID");
      //if (attr) {
      //  name = attr.as_string();
      //} else {
      //  errors.push_back("Material #" + std::to_string(material_count) + " does not have an ID");
      //  success = false;
      //  continue;
      //}

      int seconds;
      auto node = el.child("Time"); // Assume time is in seconds
      if (node) {
        seconds = node.text().as_int();
      } else {
        errors.push_back("FlowResult #" + std::to_string(count) + " does not have an associated time");
        success = false;
        continue;
      }

      node = el.child("Flows");
      if (node) {
        int flow_count{ 0 };
        std::vector<Flow<L>> flows;
        for (pugi::xml_node f : node.children("Flow")) {
          ++flow_count;
          double flow = f.text().as_double();
          auto attr = f.attribute("IDref");
          if (attr) {
            std::string name = attr.as_string();
            //std::cout << name << std::endl;
            bool found{ false };
            for (auto& el : links) {
              if (el.name == name) {
                found = true;
                flows.emplace_back(el, flow);
                break;
              }
            }
            if (!found) {
              errors.push_back("FlowResult #" + std::to_string(count) + ", Flow #" + std::to_string(flow_count) + " is linked to nonexistent object");
            }
          } else {
            errors.push_back("FlowResult #" + std::to_string(count) + ", Flow #" + std::to_string(flow_count) + " does not have a linked object");
            success = false;
            continue;
          }
        }
        //std::cout << flows.size() << std::endl;
        link_flows.emplace_back(seconds, flows);
      }

      //materials.emplace_back(name, default_conc);

    }

    return success;
  }
};

}

#endif // !AIRFLOWNETWORK_RESULTS_HPP
