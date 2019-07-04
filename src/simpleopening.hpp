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
#ifndef AIRFLOWNETWORK_SIMPLEOPENING_HPP
#define AIRFLOWNETWORK_SIMPLEOPENING_HPP

#include "element.hpp"
#include "properties.hpp"

namespace airflownetwork {

//double validate_coefficient(double v);
//double validate_exponent(double v, double default);
//double validate_pressure(double v, double default);
//double validate_temperature(double v, double default);
//double validate_humidity_ratio(double v, double default);

template <typename L, typename P> struct SimpleOpening : public Element<L,P> // Surface crack component
{
  
  const double coefficient;  // Air Mass Flow Coefficient [kg/s at 1Pa]
  const double exponent;     // Air Mass Flow exponent [dimensionless]
  const double min_density_difference;
  const double discharge_coefficient;
  const double referenceP;   // Reference barometric pressure for crack data
  const double referenceT;   // Reference temperature for crack data
  const double referenceW;   // Reference humidity ratio for crack data

  SimpleOpening(const std::string &name, double min_diff, double discharge_coeff, double coefficient, double exponent=0.65, double referenceP=101325.0, double referenceT=20.0,
    double referenceW=0.0) : Element(name), coefficient(validate_coefficient(coefficient)), exponent(validate_exponent(exponent,0.65)), 
    min_density_difference(validate_coefficient(min_diff))discharge_coefficient(validate_coefficient(discharge_coeff))
    referenceP(validate_pressure(referenceP, 101325.0)), referenceT(validate_pressure(referenceT, 20.0)),
    referenceW(validate_pressure(referenceW, 0.0))
  {
    m_viscz_norm = P::viscosity(referenceT);
    m_rhoz_norm = P::density(referenceP, referenceT, referenceW);
  }

  int calculate(bool const laminar,  // Initialization flag.If = 1, use laminar relationship
    double const pdrop,              // Total pressure drop across a component (P1 - P2) [Pa]
    L& linkage,                      // Linkage reference
    const State<P>& propN,           // Node 1 properties
    const State<P>& propM,           // Node 2 properties
    std::array<double, 2> & F,       // Airflow through the component [kg/s]
    std::array<double, 2> & DF       // Partial derivative:  DF/DP
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
    // This subroutine solves airflow for a Doorway airflow component using standard interface.
    // A doorway may have two-way airflows. Heights measured relative to the bottom of the door.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // na

    // SUBROUTINE PARAMETER DEFINITIONS:
    double const SQRT2(1.414213562373095);

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    double DPMID; // pressure drop at mid-height of doorway.
    double C;
    double DF0;   // derivative factor at the bottom of the door.
    double DFH;   // derivative factor at the top of the door.

    double F0;    // flow factor at the bottom of the door.
    double FH;    // flow factor at the top of the door.
    double Y;     // height of neutral plane rel. to bottom of door (m).

    int NF{ 1 };

    // Formats
    // static gio::Fmt Format_900("(A5,9X,4E16.7)");
    // static gio::Fmt Format_903("(A5,3I3,4E16.7)");

    // Move this all to a per-link precalculation?
    double Width{ linkage.width };
    double Height{ linkage.height };
    double coeff = coefficient * 2.0 * (Width + Height);
    double OpenFactor{ linkage.opening_factor };
    double multiplier{ linkage.multiplier };
    if (OpenFactor > 0.0) {
      Width *= OpenFactor;
      if (linkage.tilt < 90.0) {
        Height *= linkage.sin_tilt;
      }
    }

    if (pdrop >= 0.0) {
      coeff /= propN.sqrt_density;
    } else {
      coeff /= propM.sqrt_density;
    }

    // Add window multiplier with window close
    if (multiplier > 1.0) coeff *= multiplier;
    // Add window multiplier with window open
    if (OpenFactor > 0.0) {
      if (multiplier > 1.0) Width *= multiplier;
    }

    double DRHO{ propN.density - propM.density }; // difference in air densities between rooms.
    double GDRHO{ 9.8 * DRHO };

    if (OpenFactor == 0.0) {
      genericCrack(laminar, coeff, exponent, pdrop, propN, propM, F, DF);
      return 1;
    }
    if (std::abs(DRHO) < MinRhoDiff || laminar) {
      DPMID = pdrop - 0.5 * Height * GDRHO;
      // Initialization or identical temps: treat as one-way flow.
      genericCrack(laminar, coeff, exponent, DPMID, propN, propM, F, DF);
    } else {
      // Possible two-way flow:
      Y = pdrop / GDRHO;

      // F0 = lower flow, FH = upper flow.
      C = SQRT2 * Width * DischCoeff;
      DF0 = C * std::sqrt(std::abs(pdrop)) / std::abs(GDRHO);
      //        F0 = 0.666667d0*C*SQRT(ABS(GDRHO*Y))*ABS(Y)
      F0 = (2.0 / 3.0) * C * std::sqrt(std::abs(GDRHO * Y)) * std::abs(Y);
      DFH = C * std::sqrt(std::abs((Height - Y) / GDRHO));
      //        FH = 0.666667d0*DFH*ABS(GDRHO*(Height-Y))
      FH = (2.0 / 3.0) * DFH * std::abs(GDRHO * (Height - Y));

      if (Y <= 0.0) {
        // One-way flow (negative).
        if (DRHO >= 0.0) {
          F[0] = -propM.sqrt_density * std::abs(FH - F0);
          DF[0] = propM.sqrt_density * std::abs(DFH - DF0);
        } else {
          F[0] = propN.sqrt_density * std::abs(FH - F0);
          DF[0] = propN.sqrt_density * std::abs(DFH - DF0);
        }
      } else if (Y >= Height) {
        // One-way flow (positive).
        if (DRHO >= 0.0) {
          F[0] = propN.sqrt_density * std::abs(FH - F0);
          DF[0] = propN.sqrt_density * std::abs(DFH - DF0);
        } else {
          F[0] = -propM.sqrt_density * std::abs(FH - F0);
          DF[0] = propM.sqrt_density * std::abs(DFH - DF0);
        }
      } else {
        // Two-way flow.
        NF = 2;
        if (DRHO >= 0.0) {
          F[0] = -propM.sqrt_density * FH;
          DF[0] = propM.sqrt_density * DFH;
          F[1] = propN.sqrt_density * F0;
          DF[1] = propN.sqrt_density * DF0;
        } else {
          F[0] = propN.sqrt_density * FH;
          DF[0] = propN.sqrt_density * DFH;
          F[1] = -propM.sqrt_density * F0;
          DF[1] = propM.sqrt_density * DF0;
        }
      }
    }

    return NF;
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


}

#endif // !AIRFLOWNETWORK_SIMPLEOPENING_HPP
