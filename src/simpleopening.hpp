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

#include "powerlaw.hpp"
#include "properties.hpp"

namespace airflownetwork {


template <typename P> struct BasicOpening : public PowerLaw<P> // Very basic opening component
{
  const double height;
  const double width;
  const double discharge_coefficient;

  BasicOpening(const std::string &name, double height, double width, double min_diff, double discharge_coeff, double coefficient, double laminar_coefficient,
    double exponent=0.65, double referenceP=101325.0, double referenceT=20.0, double referenceW=0.0) : 
    PowerLaw<P>(name, coefficient, laminar_coefficient, exponent, referenceP, referenceT, referenceW), height(height), width(width),
    discharge_coefficient(validate_coefficient(discharge_coeff))
  {}

  virtual int calculate(bool const laminar,  // Initialization flag.If = 1, use laminar relationship
    double const pdrop,                      // Total pressure drop across a component (P1 - P2) [Pa]
    double multiplier,                       // Element multiplier
    double control,                          // Control signal
    const State<P>& propN,                   // Node 1 properties
    const State<P>& propM,                   // Node 2 properties
    std::array<double, 2> & F,               // Airflow through the component [kg/s]
    std::array<double, 2> & DF               // Partial derivative:  DF/DP
  ) const
  {

    // SUBROUTINE INFORMATION:
    //       AUTHOR         George Walton
    //       DATE WRITTEN   Extracted from AIRNET
    //       MODIFIED       Lixing Gu, 2/1/04
    //                      Revised the subroutine to meet E+ needs
    //       MODIFIED       Lixing Gu, 6/8/05
    //       RE-ENGINEERED  Jason DeGraw, 7/9/2019

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
    double Width{ width };
    double Height{ height };
    double coeff{ this->coefficient };

    if (pdrop >= 0.0) {
      coeff /= propN.sqrt_density;
    } else {
      coeff /= propM.sqrt_density;
    }

    if (control == 0.0) { // The window is closed
      generic_crack(laminar, coeff * 2.0 * (width + height), this->exponent, pdrop, propN, propM, F, DF);
      return 1;
    }

    //double coeff = coefficient*2.0 * (Width + Height); // This has consequences for the open laminar case, the crack length should not be involved
    //double OpenFactor{ control };
    
    if (control > 0.0) {
      Width *= control;
      //if (linkage.tilt < 90.0) {
      //  Height *= linkage.sin_tilt;
      //}
    }

    //if (pdrop >= 0.0) {
    //  coeff /= propN.sqrt_density;
    //} else {
    //  coeff /= propM.sqrt_density;
    //}

    // Add window multiplier with window close
    if (multiplier > 1.0) coeff *= multiplier;
    // Add window multiplier with window open
    if (control > 0.0) {
      if (multiplier > 1.0) Width *= multiplier;
    }

    double DRHO{ propN.density - propM.density }; // difference in air densities between rooms.
    double GDRHO{ 9.8 * DRHO };

    //if (OpenFactor == 0.0) {
    //  generic_crack(laminar, coeff, exponent, pdrop, propN, propM, F, DF);
    //  return 1;
    //}
    if (std::abs(DRHO) <= 0.0001 * std::abs(pdrop)) {
      DPMID = pdrop - 0.5 * Height * GDRHO;
      // Initialization or identical temps: treat as one-way flow.
      generic_crack(laminar, coeff, this->exponent, DPMID, propN, propM, F, DF);
    } else {
      // Possible two-way flow:
      Y = pdrop / GDRHO;

      // F0 = lower flow, FH = upper flow.
      C = SQRT2 * Width * discharge_coefficient;
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

private:
  // Nothing to see here

};


template <typename P> struct SimpleOpening : public PowerLaw<P> // Simple opening component
{
  const double height;
  const double width;
  const double min_density_difference;
  const double discharge_coefficient;

  SimpleOpening(const std::string &name, double height, double width, double min_diff, double discharge_coeff, double coefficient, double laminar_coefficient,
    double exponent=0.65, double referenceP=101325.0, double referenceT=20.0, double referenceW=0.0) : 
    PowerLaw<P>(name, coefficient, laminar_coefficient, exponent, referenceP, referenceT, referenceW), height(height), width(width),
    min_density_difference(validate_coefficient(min_diff)), discharge_coefficient(validate_coefficient(discharge_coeff))
  {}

  virtual int calculate(bool const laminar,  // Initialization flag.If = 1, use laminar relationship
    double const pdrop,                      // Total pressure drop across a component (P1 - P2) [Pa]
    double multiplier,                       // Element multiplier
    double control,                          // Control signal
    const State<P>& propN,                   // Node 1 properties
    const State<P>& propM,                   // Node 2 properties
    std::array<double, 2> & F,               // Airflow through the component [kg/s]
    std::array<double, 2> & DF               // Partial derivative:  DF/DP
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
    double Width{ width };
    double Height{ height };
    double coeff = this->coefficient * 2.0 * (Width + Height); // This has consequences for the open laminar case, the crack length should not be involved
    double OpenFactor{ control };
    
    if (OpenFactor > 0.0) {
      Width *= OpenFactor;
      //if (linkage.tilt < 90.0) {
      //  Height *= linkage.sin_tilt;
      //}
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
      generic_crack(laminar, coeff, this->exponent, pdrop, propN, propM, F, DF);
      return 1;
    }
    if (std::abs(DRHO) < min_density_difference || laminar) {
      DPMID = pdrop - 0.5 * Height * GDRHO;
      // Initialization or identical temps: treat as one-way flow.
      generic_crack(laminar, coeff, this->exponent, DPMID, propN, propM, F, DF);
    } else {
      // Possible two-way flow:
      Y = pdrop / GDRHO;

      // F0 = lower flow, FH = upper flow.
      C = SQRT2 * Width * discharge_coefficient;
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

private:
  // Nothing to see here

};


}

#endif // !AIRFLOWNETWORK_SIMPLEOPENING_HPP
