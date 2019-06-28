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
#ifndef POWERLAW_HPP
#define POWERLAW_HPP

#include "element.hpp"
#include "properties.hpp"

namespace airflownetwork {

//double validate_coefficient(double v);
//double validate_exponent(double v, double default);
//double validate_pressure(double v, double default);
//double validate_temperature(double v, double default);
//double validate_humidity_ratio(double v, double default);

template <typename P> struct PowerLaw : public Element<P> // Surface crack component
{
  
  const double coefficient;  // Air Mass Flow Coefficient [kg/s at 1Pa]
  const double laminar_coefficient;  // "Laminar" Air Mass Flow Coefficient [kg/s at 1Pa]
  const double exponent;     // Air Mass Flow exponent [dimensionless]
  const double referenceP;   // Reference barometric pressure for crack data
  const double referenceT;   // Reference temperature for crack data
  const double referenceW;   // Reference humidity ratio for crack data

  PowerLaw(const std::string &name, double coefficient, double laminar_coefficient, double exponent=0.65, double referenceP=101325.0, double referenceT=20.0,
    double referenceW=0.0) : Element(name), coefficient(validate_coefficient(coefficient)), laminar_coefficient(validate_coefficient(laminar_coefficient)), 
    exponent(validate_exponent(exponent,0.65)), referenceP(validate_pressure(referenceP, 101325.0)), referenceT(validate_pressure(referenceT, 20.0)),
    referenceW(validate_pressure(referenceW, 0.0))
  {
    m_viscz_norm = P::viscosity(referenceT);
    m_rhoz_norm = P::density(referenceP, referenceT, referenceW);
  }

  int calculate(bool const laminar,  // Initialization flag.If = 1, use laminar relationship
    double const pdrop,              // Total pressure drop across a component (P1 - P2) [Pa]
    double multiplier,               // Linkage multiplier
    const State<P>& propN,           // Node 1 properties
    const State<P>& propM,           // Node 2 properties
    std::array<double, 2>& F,        // Airflow through the component [kg/s]
    std::array<double, 2>& DF        // Partial derivative:  DF/DP
  ) const
  {
    // SUBROUTINE INFORMATION:
    //       AUTHOR         George Walton
    //       DATE WRITTEN   Extracted from AIRNET
    //       MODIFIED       Lixing Gu, 2/1/04
    //                      Revised the subroutine to meet E+ needs
    //       MODIFIED       Lixing Gu, 6/8/05
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine solves airflow for a surface crack component

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

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

    double coef = coefficient * multiplier / upwind_sqrt_density;
    
    // Laminar calculation
    double RhoCor{ TOKELVIN(upwind_temperature) / TOKELVIN(Tave) };
    double Ctl{ std::pow(m_rhoz_norm / upwind_density / RhoCor, exponent - 1.0) * std::pow(m_viscz_norm / VisAve, 2.0 * exponent - 1.0) };
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
  return 1;
}

  double linearize(double multiplier,  // Linkage multiplier
    const State<P>& propN,             // Node 1 properties
    const State<P>& propM              // Node 2 properties
  ) const
  {
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Jason DeGraw
    //       DATE WRITTEN   Extracted from AFE*** from AIRNET
    //       MODIFIED       na
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes the linear coefficient for airflow for a surface crack component

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    double VisAve{ 0.5 * (propN.viscosity + propM.viscosity) };
    double Tave{ 0.5 * (propN.temperature + propM.temperature) };
    double Dave{ 0.5 * (propN.density + propM.density) };

    double coef = coefficient * multiplier / std::sqrt(Dave);

    // Laminar calculation
    double Ctl{ std::pow(m_rhoz_norm / Dave, exponent - 1.0) * std::pow(m_viscz_norm / VisAve, 2.0 * exponent - 1.0) };
    double CDM{ coef * Dave / VisAve * Ctl };

    return CDM;
  }

private:
double m_viscz_norm;
double m_rhoz_norm;

};


template <typename P> struct ContamXPowerLaw : public Element<P> // Surface crack component
{

  const double coefficient;  // Air Mass Flow Coefficient [kg/s at 1Pa]
  const double laminar_coefficient;  // "Laminar" Air Mass Flow Coefficient [kg/s at 1Pa]
  const double exponent;     // Air Mass Flow exponent [dimensionless]

  // Default Constructor
  ContamXPowerLaw(const std::string& name, double coefficient, double laminar_coefficient, double exponent = 0.65) : Element(name), coefficient(validate_coefficient(coefficient)),
    laminar_coefficient(validate_coefficient(laminar_coefficient)), exponent(validate_exponent(exponent, 0.65))
  {}

  static double adjustment(double density, double dynamic_viscosity, double exponent)
  {
    return exp(log(1.20410 / density) * (exponent - 0.5) + log(1.50839e-5 * dynamic_viscosity) * (2 * exponent - 1));

  }

  static double pow(double a, double x)
  {
    if (x == 0.5) {
      return std::sqrt(a);
    }
    return std::pow(a, x);
  }

  virtual int calculate(bool const laminar,  // Initialization flag.If = 1, use laminar relationship
    double const pdrop,              // Total pressure drop across a component (P1 - P2) [Pa]
    double multiplier,               // Linkage multiplier
    const State<P>& propN,           // Node 1 properties
    const State<P>& propM,           // Node 2 properties
    std::array<double, 2>& F,        // Airflow through the component [kg/s]
    std::array<double, 2>& DF        // Partial derivative:  DF/DP
  ) const
  {

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
    double dvisc = upwind_viscosity / upwind_density;

    // Laminar calculation
    double cdm{ laminar_coefficient * multiplier * dvisc };
    double FL{ cdm * pdrop };

    if (laminar) {
      DF[0] = cdm;
      F[0] = FL;
    } else {
      // Turbulent flow.
      double Tadj = adjustment(upwind_density, dvisc, exponent);
      double FT = sign * Tadj * coefficient * multiplier * std::sqrt(0.5 * (propN.density + propM.density)) * pow(abs_pdrop, exponent);

      // Select laminar or turbulent flow.
      if (std::abs(FL) <= std::abs(FT)) {
        F[0] = FL;
        DF[0] = cdm;
      } else {
        F[0] = FT;
        DF[0] = FT * exponent / pdrop;
      }
    }
    return 1;
  }

  double linearize(double multiplier,  // Linkage multiplier
    const State<P>& propN,             // Node 1 properties
    const State<P>& propM              // Node 2 properties
  ) const
  {

    double dviscN = propN.viscosity / propN.density;
    double dviscM = propM.viscosity / propM.density;
    double dvisc = 0.5 * (dviscN + dviscM);

    // Laminar calculation
    return laminar_coefficient * multiplier * dvisc;
  }

};


}

#endif // !POWERLAW_HPP
