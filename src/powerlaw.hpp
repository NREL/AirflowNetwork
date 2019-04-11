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

template <typename P> struct PowerLaw : public AirflowElement<P> // Surface crack component
{
  const double coefficient;  // Air Mass Flow Coefficient [kg/s at 1Pa]
  const double exponent;     // Air Mass Flow exponent [dimensionless]
  const double referenceP;   // Reference barometric pressure for crack data
  const double referenceT;   // Reference temperature for crack data
  const double referenceW;   // Reference humidity ratio for crack data

  // Default Constructor
  PowerLaw(const std::string &name, double coefficient, double exponent=0.65, double referenceP=101325.0, double referenceT=20.0,
    double referenceW=0.0) : AirflowElement(name), coefficient(validate_coefficient(coefficient)), exponent(validate_exponent(exponent,0.65)),
    referenceP(validate_pressure(referenceP, 101325.0)), referenceT(validate_pressure(referenceT, 20.0)),
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
      upwind_temperature = propN.temperature;
      upwind_density = propN.density;
      upwind_viscosity = propN.viscosity;
      upwind_sqrt_density = propN.sqrt_density;
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

}

#endif // !POWERLAW_HPP
