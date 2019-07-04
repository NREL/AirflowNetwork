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

template<typename P> void genericCrack(bool const laminar, // Initialization flag.If = 1, use laminar relationship
  double const coefficient,                                // Flow coefficient
  double const exponent,                                   // Flow exponent
  double const pdrop,                                      // Total pressure drop across a component (P1 - P2) [Pa]
  const State<P>& propN,                                   // Node 1 properties
  const State<P>& propM,                                   // Node 2 properties
  std::array<double, 2> & F,                               // Airflow through the component [kg/s]
  std::array<double, 2> & DF,                              // Partial derivative:  DF/DP
  double const referenceP = 101325.0,                      // Reference pressure
  double const referenceT = 20.0,                          // Reference temperature
  double const referenceW = 0.0                            // Reference humidity ratio
)
{

  // SUBROUTINE INFORMATION:
  //       AUTHOR         George Walton
  //       DATE WRITTEN   Extracted from AIRNET
  //       MODIFIED       Lixing Gu, 2/1/04
  //                      Revised the subroutine to meet E+ needs
  //       MODIFIED       Lixing Gu, 6/8/05
  //       RE-ENGINEERED  This subroutine is revised from AFEPLR developed by George Walton, NIST
  //                      Jason DeGraw

  // PURPOSE OF THIS SUBROUTINE:
  // This subroutine solves airflow for a power law component

  // METHODOLOGY EMPLOYED:
  // Using Q=C(dP)^n

  // REFERENCES:
  // na

  // FLOW:
  // Calculate normal density and viscocity at reference conditions
  double RhozNorm = P::density(referenceP, referenceT, referenceW);
  //VisczNorm = 1.71432e-5 + 4.828e-8 * 20.0;
  double VisczNorm = P::viscosity(referenceT);

  double VisAve{ 0.5 * (propN.viscosity + propM.viscosity) };
  double Tave{ 0.5 * (propN.temperature + propM.temperature) };

  double sign{ 1.0 };
  double upwind_temperature{ propN.temperature };
  double upwind_density{ propN.density };
  double upwind_viscosity{ propN.viscosity };
  double upwind_sqrt_density{ propN.sqrt_density };
  double abs_pdrop = pdrop;

  if (pdrop < 0.0) {
    sign = -1.0;
    upwind_temperature = propM.temperature;
    upwind_density = propM.density;
    upwind_viscosity = propM.viscosity;
    upwind_sqrt_density = propM.sqrt_density;
    abs_pdrop = -pdrop;
  }

  double coef = coefficient/upwind_sqrt_density;

  // Laminar calculation
  double RhoCor{ TOKELVIN(upwind_temperature) / TOKELVIN(Tave) };
  double Ctl{ std::pow(RhozNorm / upwind_density / RhoCor, exponent - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * exponent - 1.0) };
  double CDM{ coef * upwind_density / upwind_viscosity * Ctl };
  double FL{ CDM * pdrop };
  double FT;

  if (laminar) {
    DF[0] = CDM;
    F[0] = FL;
  } else {
    // Turbulent flow.
    if (exponent == 0.5) {
      FT = sign * coef * upwind_sqrt_density * std::sqrt(abs_pdrop) * Ctl;
    } else {
      FT = sign * coef * upwind_sqrt_density * std::pow(abs_pdrop, exponent) * Ctl;
    }
    // Select laminar or turbulent flow.
    if (std::abs(FL) <= std::abs(FT)) {
      F[0] = FL;
      DF[0] = CDM;
    } else {
      F[0] = FT;
      DF[0] = FT * exponent / pdrop;
    }
  }

  return;
}

template<typename P> void genericCrack0(bool const laminar, // Initialization flag.If = 1, use laminar relationship
  double const coefficient,                                 // Flow coefficient
  double const exponent,                                    // Flow exponent
  double const PDROP,                                       // Total pressure drop across a component (P1 - P2) [Pa]
  const State<P>& propN,                                    // Node 1 properties
  const State<P>& propM,                                    // Node 2 properties
  std::array<double, 2> & F,                                // Airflow through the component [kg/s]
  std::array<double, 2> & DF,                               // Partial derivative:  DF/DP
  double const referenceP = 101325.0,                       // Reference pressure
  double const referenceT = 20.0,                           // Reference temperature
  double const referenceW = 0.0                             // Reference humidity ratio
)
{

  // SUBROUTINE INFORMATION:
  //       AUTHOR         George Walton
  //       DATE WRITTEN   Extracted from AIRNET
  //       MODIFIED       Lixing Gu, 2/1/04
  //                      Revised the subroutine to meet E+ needs
  //       MODIFIED       Lixing Gu, 6/8/05
  //       RE-ENGINEERED  This subroutine is revised from AFEPLR developed by George Walton, NIST

  // PURPOSE OF THIS SUBROUTINE:
  // This subroutine solves airflow for a power law component

  // METHODOLOGY EMPLOYED:
  // Using Q=C(dP)^n

  // REFERENCES:
  // na

  // USE STATEMENTS:
  // na

  // Locals
  // SUBROUTINE ARGUMENT DEFINITIONS:

  // SUBROUTINE PARAMETER DEFINITIONS:
  // na

  // INTERFACE BLOCK SPECIFICATIONS
  // na

  // DERIVED TYPE DEFINITIONS
  // na

  // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
  double CDM;
  double FL;
  double FT;
  double Ctl;
  double VisAve;
  double Tave;
  double RhoCor;


  // FLOW:
  // Calculate normal density and viscocity at Crack standard condition: T=20C, p=101325 Pa and 0 g/kg
  double RhozNorm = P::density(referenceP, referenceT, referenceW);
  //VisczNorm = 1.71432e-5 + 4.828e-8 * 20.0;
  double VisczNorm = P::viscosity(referenceT);
  VisAve = (propN.viscosity + propM.viscosity) / 2.0;
  Tave = (propN.temperature + propM.temperature) / 2.0;
  double coef = coefficient;
  if (PDROP >= 0.0) {
    coef /= propN.sqrt_density;
  } else {
    coef /= propM.sqrt_density;
  }

  if (laminar) {
    // Initialization by linear relation.
    if (PDROP >= 0.0) {
      RhoCor = TOKELVIN(propN.temperature) / TOKELVIN(Tave);
      Ctl = std::pow(RhozNorm / propN.density / RhoCor, exponent - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * exponent - 1.0);
      DF[0] = coef * propN.density / propN.viscosity * Ctl;
    } else {
      RhoCor = TOKELVIN(propM.temperature) / TOKELVIN(Tave);
      Ctl = std::pow(RhozNorm / propM.density / RhoCor, exponent - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * exponent - 1.0);
      DF[0] = coef * propM.density / propM.viscosity * Ctl;
    }
    F[0] = DF[0] * PDROP;
  } else {
    // Standard calculation.
    if (PDROP >= 0.0) {
      // Flow in positive direction.
      // Laminar flow.
      RhoCor = TOKELVIN(propN.temperature) / TOKELVIN(Tave);
      Ctl = std::pow(RhozNorm / propN.density / RhoCor, exponent - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * exponent - 1.0);
      CDM = coef * propN.density / propN.viscosity * Ctl;
      FL = CDM * PDROP;
      // Turbulent flow.
      if (exponent == 0.5) {
        FT = coef * propN.sqrt_density * std::sqrt(PDROP) * Ctl;
      } else {
        FT = coef * propN.sqrt_density * std::pow(PDROP, exponent) * Ctl;
      }
    } else {
      // Flow in negative direction.
      // Laminar flow.
      RhoCor = TOKELVIN(propM.temperature) / TOKELVIN(Tave);
      Ctl = std::pow(RhozNorm / propM.density / RhoCor, 2.0 * exponent - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * exponent - 1.0);
      CDM = coef * propM.density / propM.viscosity * Ctl;
      FL = CDM * PDROP;
      // Turbulent flow.
      if (exponent == 0.5) {
        FT = -coef * propM.sqrt_density * std::sqrt(-PDROP) * Ctl;
      } else {
        FT = -coef * propM.sqrt_density * std::pow(-PDROP, exponent) * Ctl;
      }
    }
    // Select laminar or turbulent flow.
    if (std::abs(FL) <= std::abs(FT)) {
      F[0] = FL;
      DF[0] = CDM;
    } else {
      F[0] = FT;
      DF[0] = FT * exponent / PDROP;
    }
  }
  return;
}

template <typename L, typename P> struct Element
{
  Element(const std::string& name) : name(name)
  {}

  const std::string name;

  virtual int calculate(bool laminar,  // Initialization flag.If = 1, use laminar relationship
    double pdrop,                      // Total pressure drop across a component (P1 - P2) [Pa]
    L& linkage,                        // Linkage reference
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
