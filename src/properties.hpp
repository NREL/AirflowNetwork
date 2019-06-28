// EnergyPlus, Copyright (c) 1996-2019, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
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
  State() : temperature(P::temperature_0), pressure(P::pressure_0), humidity_ratio(P::humidity_ratio_0),
    density(P::density(pressure, temperature, humidity_ratio)), sqrt_density(std::sqrt(P::density(pressure, temperature, humidity_ratio))),
    viscosity(P::viscosity(temperature))
  {}

  State(double pressure, double temperature, double humidity_ratio) : temperature(temperature), pressure(pressure), humidity_ratio(humidity_ratio),
    density(P::density(pressure, temperature, humidity_ratio)), sqrt_density(std::sqrt(density)),
    viscosity(P::viscosity(temperature))
  {}

  double temperature{ P::temperature_0 };
  double pressure{ P::pressure_0 };  // absolute pressure
  double humidity_ratio{ P::humidity_ratio_0 };
  double density{ P::density(P::pressure_0, P::temperature_0, P::humidity_ratio_0) };
  double sqrt_density{ sqrt(P::density(P::pressure_0, P::temperature_0, P::humidity_ratio_0)) };
  double viscosity{ P::viscosity(P::temperature_0) };
};

}

#endif // !PROPERTIES_HPP
