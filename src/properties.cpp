#include "properties.hpp"
namespace airflownetwork {

double ideal_gas_density(double P, double T, double)
{
  return P / (reference::R * (T + 273.15));
}

inline double airnet_density(double P, double T, double)
{
  return 0.0034838* P / (T + 273.15);
}


double airThermConductivity(double T // Temperature in Celsius
)
{
  // Dry air thermal conductivity {W/m-K}
  // Correlated over the range -20C to 70C
  // Reference Cengel & Ghajar, Heat and Mass Transfer. 5th ed.

  double const LowerLimit = -20;
  double const UpperLimit = 70;

  double const a = 0.02364;
  double const b = 0.0000754772569209165;
  double const c = -2.40977632412045e-8;

  if (T < LowerLimit) {
    //ShowWarningMessage("Air temperature below lower limit of -20C for conductivity calculation");
    T = LowerLimit;
  } else if (T > UpperLimit) {
    //ShowWarningMessage("Air temperature above upper limit of 70C for conductivity calculation");
    T = UpperLimit;
  }

  return a + b * T + c * T * T;
}

double sutherland_dynamic_viscosity(double T)
{
  double TK{ T + 273.15 };
  return 6.558705e-3* pow(T / 273.15, 1.5) / (T + 110.4);
}

double airDynamicVisc(double T // Temperature in Celsius
)
{
  return 1.71432e-5 + 4.828e-8 * T;
}

double airKinematicVisc(double T, // Temperature in Celsius
  double W, // Humidity ratio
  double P  // Barometric pressure
)
{
  // Dry air kinematic viscosity {m2/s}
  // Correlated over the range -20C to 70C
  // Reference Cengel & Ghajar, Heat and Mass Transfer. 5th ed.

  double const LowerLimit = -20;
  double const UpperLimit = 70;

  if (T < LowerLimit) {
    T = LowerLimit;
  } else if (T > UpperLimit) {
    T = UpperLimit;
  }

  return airDynamicVisc(T) / ideal_gas_density(P, T, W); // need E+ implementation
}

/*
double airThermalDiffusivity(double T, // Temperature in Celsius
  double W, // Humidity ratio
  double P  // Barometric pressure
)
{
  // Dry air thermal diffusivity {-}
  // Correlated over the range -20C to 70C
  // Reference Cengel & Ghajar, Heat and Mass Transfer. 5th ed.

  double const LowerLimit = -20;
  double const UpperLimit = 70;

  if (T < LowerLimit) {
    T = LowerLimit;
  } else if (T > UpperLimit) {
    T = UpperLimit;
  }

  return airThermConductivity(T) / (AIRCP(W, T) * AIRDENSITY(P, T, W));
}

double airPrandtl(double T, // Temperature in Celsius
  double W, // Humidity ratio
  double P  // Barometric pressure
)
{
  // Dry air Prandtl number {-}
  // Correlated over the range -20C to 70C
  // Reference Cengel & Ghajar, Heat and Mass Transfer. 5th ed.

  double const LowerLimit = -20;
  double const UpperLimit = 70;

  if (T < LowerLimit) {
    T = LowerLimit;
  } else if (T > UpperLimit) {
    T = UpperLimit;
  }

  return airKinematicVisc(T, W, P) / airThermalDiffusivity(T, W, P);
}
*/


}