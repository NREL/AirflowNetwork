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
#ifndef AIRFLOWNETWORK_ELEMENT_HPP
#define AIRFLOWNETWORK_ELEMENT_HPP

#include <string>
#include <array>
#include "properties.hpp"

#define TOKELVIN(T) (T+273.15)

namespace airflownetwork {

inline double validate_coefficient(double v)
{
  if (v == 0.0) {
    return 1.0;
  }
  return std::abs(v);
}

inline double validate_exponent(double v, double default)
{
  if (v < 0.5 || v > 1.0) {
    return default;
  }
  return v;
}

inline double validate_pressure(double v, double default)
{
  if (v <= 0.0) {
    return default;
  }
  return v;
}

inline double validate_temperature(double v, double default)
{
  if (v <= -273.15) {
    return default;
  }
  return v;
}

inline double validate_humidity_ratio(double v, double default)
{
  if (v <= 0.0) {
    return default;
  }
  return v;
}

template <typename P> struct Element
{
  Element(const std::string& name) : name(name)
  {}

  const std::string name;

  virtual int calculate(bool laminar,  // Initialization flag.If = 1, use laminar relationship
    double pdrop,                      // Total pressure drop across a component (P1 - P2) [Pa]
    double multiplier,                 // Linkage multiplier
    const State<P>& propN,             // Node 1 properties
    const State<P>& propM,             // Node 2 properties
    std::array<double, 2>& F,          // Airflow through the component [kg/s]
    std::array<double, 2>& DF          // Partial derivative:  DF/DP
  ) const = 0;

  virtual double linearize(double multiplier,  // Linkage multiplier
    const State<P>& propN,                     // Node 1 properties
    const State<P>& propM                      // Node 2 properties
  ) const = 0;
};

}

#endif // !AIRFLOWNETWORK_ELEMENT_HPP
