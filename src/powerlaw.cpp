#include "node.hpp"
#include "powerlaw.hpp"
#include "properties.hpp"

namespace airflownetwork {
/*
double validate_coefficient(double v)
{
  if (v == 0.0) {
    return 1.0;
  }
  return std::abs(v);
}

double validate_exponent(double v, double default)
{
  if (v < 0.5 || v > 1.0) {
    return default;
  }
  return v;
}

double validate_pressure(double v, double default)
{
  if (v <= 0.0) {
    return default;
  }
  return v;
}

double validate_temperature(double v, double default)
{
  if (v <= -273.15) {
    return default;
  }
  return v;
}

double validate_humidity_ratio(double v, double default)
{
  if (v <= 0.0) {
    return default;
  }
  return v;
}
*/
/*
int PowerLaw::calculate(bool const laminar, double const pdrop, double multiplier, const State& propN, const State& propM, std::array<double, 2>& F, std::array<double, 2>& DF)
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
    
    double RhozNorm = density(referenceP, referenceT, referenceW);
    double VisczNorm = 1.71432e-5 + 4.828e-8 * referenceT;

    double VisAve{ 0.5 * (propN.viscosity + propM.viscosity) };
    double Tave{ 0.5 * (propN.temperature + propM.temperature) };

    double sign{ 1.0 };
    double upwind_temperature{ propN.density };
    double upwind_density{ propN.density };
    double upwind_viscosity{ propN.viscosity };
    double upwind_sqrt_density{ propN.sqrt_density };
    double abs_pdrop = pdrop;

    if (pdrop < 0.0) {
      sign = -1.0;
      upwind_temperature = propN.density;
      upwind_density = propN.density;
      upwind_viscosity = propN.viscosity;
      upwind_sqrt_density = propN.sqrt_density;
      abs_pdrop = -pdrop;
    }

    double coef = coefficient * multiplier / upwind_sqrt_density;
    
    // Laminar calculation
    double RhoCor{ TOKELVIN(upwind_temperature) / TOKELVIN(Tave) };
    double Ctl{ std::pow(RhozNorm / upwind_density / RhoCor, exponent - 1.0) * std::pow(VisczNorm / VisAve, 2.0 * exponent - 1.0) };
    double CDM{ coef * upwind_density / upwind_viscosity * Ctl };
    double FL{ CDM * pdrop };
    double FT;

    if (laminar) {
      DF[0] = CDM;
      F[0] = -DF[0] * pdrop;
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
*/
}
