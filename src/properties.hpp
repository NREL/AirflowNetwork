#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP

#include <algorithm>

namespace airflownetwork {

namespace properties{

struct EnergyPlus
{
  static constexpr double temperature_0 = 20.0;
  static constexpr double pressure_0 = 101325.0;
  static constexpr double humidity_ratio_0 = 0.0;
  static constexpr double R_0 = 287.0; // Should check that this is used everywhere

  static double density(double P, double T, double w)
  {
    return P / (287.0 * (T + 273.15) * (1.0 + 1.6077687 * std::max(w, 1.0e-5)));
  }

  static double viscosity(double T)
  {
    return 1.71432e-5 + 4.828E-8 * T;
  }
};

struct AIRNET
{
  static constexpr double temperature_0 = 20.0;
  static constexpr double pressure_0 = 101325.0;
  static constexpr double humidity_ratio_0 = 0.0;
  static constexpr double R_0 = 287.055; // Maybe should adjust this to match better

  static double density(double P, double T, double)
  {
    return 0.0034838* P / (T + 273.15);
  }

  static double viscosity(double T)
  {
    return 1.71432e-5 + 4.828E-8 * T;
  }
};

struct CONTAM
{
  static constexpr double temperature_0 = 20.0;
  static constexpr double pressure_0 = 101325.0;
  static constexpr double humidity_ratio_0 = 0.0;
  static constexpr double R_0 = 287.055;

  static double density(double P, double T, double)
  {
    return P / (R_0 * (T + 273.15));
  }

  static double viscosity(double T)
  {
    return 3.7143e-6 + 4.9286e-8 * T;
  }
};

struct Fixed
{
  static constexpr double temperature_0 = 20.0;
  static constexpr double pressure_0 = 101325.0;
  static constexpr double humidity_ratio_0 = 0.0;
  static constexpr double R_0 = 287.055; // Maybe should adjust this to match better
  
  static double density(double, double, double)
  {
    return 1.2041;
  }

  static double viscosity(double)
  {
    return 0.0000181625;
  }

  static double kinematic_viscosity(double, double, double)
  {
    return 0.0000181625/1.2041;
  }
};

}

namespace reference {
const double P = 101325.0;
const double T = 293.15;
const double R = 287.055;
}

/** Density calculated from the ideal gas equation of state
*/
double ideal_gas_density(double p,  /**< pressure (in Pa) */
  double T, /**< temperature (in K) */
  double w  /**< humidity ratio */
);

/** Density as calculated by AIRNET
*/
inline double airnet_density(double P, /**< pressure (in Pa) */
  double T, /**< temperature (in K) */
  double w  /**< humidity ratio */
);

double airThermConductivity(double T // Temperature in Celsius
);

/** Sutherland's law for the dynamic viscosity of air
*/
double sutherland_dynamic_viscosity(double T /**< temperature (in C) */
);

double airDynamicVisc(double T // Temperature in Celsius
);

double airKinematicVisc(double T, // Temperature in Celsius
  double W, // Humidity ratio
  double P  // Barometric pressure
);

//double airThermalDiffusivity(double T, // Temperature in Celsius
//  double W, // Humidity ratio
//  double P  // Barometric pressure
//);

//double airPrandtl(double T, // Temperature in Celsius
//  double W, // Humidity ratio
//  double P  // Barometric pressure
//);


//auto& density{ ideal_gas_density };
//auto& dynamic_viscosity{ sutherland_dynamic_viscosity };

template <typename P> struct State
{
  double temperature{ P::temperature_0 };
  // double pressure;      //{0.0}; // gage pressure
  double humidityRatio{ P::humidity_ratio_0 };
  double density{ P::density(P::pressure_0, P::temperature_0, P::humidity_ratio_0) };
  double sqrt_density{ sqrt(P::density(P::pressure_0, P::temperature_0, P::humidity_ratio_0)) };
  double viscosity{ P::viscosity(P::temperature_0) };
};

}

#endif // !PROPERTIES_HPP
