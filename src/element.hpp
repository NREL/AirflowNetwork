#ifndef AIRFLOWELEMENT_HPP
#define AIRFLOWELEMENT_HPP

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

template <typename P> struct AirflowElement
{
  AirflowElement(const std::string& name) : name(name)
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

#endif // !AIRFLOWELEMENT_HPP
