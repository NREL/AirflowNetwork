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
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "powerlaw.hpp"
#include "simpleopening.hpp"

TEST_CASE("Test the generic crack function", "[genericCrack]")
{
  airflownetwork::State<airflownetwork::properties::Fixed> state0;
  airflownetwork::State<airflownetwork::properties::Fixed> state1;

  std::array<double, 2> F{ {0.0, 0.0} };
  std::array<double, 2> DF{ {0.0, 0.0} };

  double dp{ 10.0 };

  // Linearized test
  //double C = powerlaw.linearize(1.0, state0, state1);
  //CHECK(C == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));

  // Laminar tests
  airflownetwork::generic_crack(true, 0.001, 0.65, dp, state0, state1, F, DF);
  CHECK(F[0] == .01 * std::sqrt(1.2041) / 0.0000181625);
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));
  CHECK(DF[1] == 0.0);

  airflownetwork::generic_crack(true, 0.001, 0.65, -dp, state0, state1, F, DF);
  CHECK(F[0] == -.01 * std::sqrt(1.2041) / 0.0000181625);
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));
  CHECK(DF[1] == 0.0);

  // Turbulent tests
  airflownetwork::generic_crack(false, 0.001, 0.65, dp, state0, state1, F, DF);
  CHECK(F[0] == .001 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000065 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);

  airflownetwork::generic_crack(false, 0.001, 0.65, -dp, state0, state1, F, DF);
  CHECK(F[0] == -.001 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000065 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);
}

TEST_CASE("Test the original generic crack function", "[genericCrack0]")
{
  airflownetwork::State<airflownetwork::properties::Fixed> state0;
  airflownetwork::State<airflownetwork::properties::Fixed> state1;

  std::array<double, 2> F{ {0.0, 0.0} };
  std::array<double, 2> DF{ {0.0, 0.0} };

  double dp{ 10.0 };

  // Linearized test
  //double C = powerlaw.linearize(1.0, state0, state1);
  //CHECK(C == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));

  // Laminar tests
  airflownetwork::genericCrack0(true, 0.001, 0.65, dp, state0, state1, F, DF);
  CHECK(F[0] == .01 * std::sqrt(1.2041) / 0.0000181625);
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));
  CHECK(DF[1] == 0.0);

  airflownetwork::genericCrack0(true, 0.001, 0.65, -dp, state0, state1, F, DF);
  CHECK(F[0] == -.01 * std::sqrt(1.2041) / 0.0000181625);
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));
  CHECK(DF[1] == 0.0);

  // Turbulent tests
  airflownetwork::genericCrack0(false, 0.001, 0.65, dp, state0, state1, F, DF);
  CHECK(F[0] == .001 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000065 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);

  airflownetwork::genericCrack0(false, 0.001, 0.65, -dp, state0, state1, F, DF);
  CHECK(F[0] == -.001 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000065 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);
}

TEST_CASE("Test the power law element", "[PowerLaw]")
{
  airflownetwork::PowerLaw<airflownetwork::properties::Fixed> powerlaw("powerlaw", 0.001, 0.001);
  airflownetwork::State<airflownetwork::properties::Fixed> state0;
  airflownetwork::State<airflownetwork::properties::Fixed> state1;

  std::array<double, 2> F{ {0.0, 0.0} };
  std::array<double, 2> DF{ {0.0, 0.0} };

  double multiplier{ 1.0 };
  double control{ 1.0 };

  double dp{ 10.0 };

  // Linearized test
  double C = powerlaw.linearize(1.0, state0, state1);
  CHECK(C == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));

  // Laminar tests
  powerlaw.calculate(true, dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == .01 * std::sqrt(1.2041) / 0.0000181625);
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));
  CHECK(DF[1] == 0.0);

  powerlaw.calculate(true, -dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == -.01 * std::sqrt(1.2041) / 0.0000181625);
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));
  CHECK(DF[1] == 0.0);

  // Turbulent tests
  powerlaw.calculate(false, dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == .001 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000065 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);

  powerlaw.calculate(false, -dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == -.001 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000065 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);

  control = 2.0;
  powerlaw.calculate(false, dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == .002 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.00013 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);

  powerlaw.calculate(false, -dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == -.002 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.00013 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);
}

TEST_CASE("Test the simple opening element", "[SimpleOpening]")
{
  airflownetwork::SimpleOpening<airflownetwork::properties::Fixed> opening("opening", 1.0, 0.5, 0.01, 0.5, 0.001, 0.001);

  airflownetwork::State<airflownetwork::properties::Fixed> state0;
  airflownetwork::State<airflownetwork::properties::Fixed> state1;

  std::array<double, 2> F{ {0.0, 0.0} };
  std::array<double, 2> DF{ {0.0, 0.0} };

  double multiplier{ 1.0 };
  double control{ 0.0 };

  double dp{ 10.0 };

  // Crack tests, laminar
  opening.calculate(true, dp, multiplier, control, state0, state1, F, DF);
  //CHECK(F[0] == .01 * std::sqrt(1.2041) / 0.0000181625);
  CHECK(F[0] == Approx(.03 / 0.0000181625));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.003 / 0.0000181625));
  CHECK(DF[1] == 0.0);

  opening.calculate(true, -dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == Approx(-.03 / 0.0000181625));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.003 / 0.0000181625));
  CHECK(DF[1] == 0.0);


  // Crack tests, turbulent
  opening.calculate(false, dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == .003 * std::pow(10.0, 0.65) / std::sqrt(1.2041));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000195 * std::pow(10.0, 0.65) / std::sqrt(1.2041)));
  CHECK(DF[1] == 0.0);

  opening.calculate(false, -dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == -.003 * std::pow(10.0, 0.65) / std::sqrt(1.2041));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000195 * std::pow(10.0, 0.65) / std::sqrt(1.2041)));
  CHECK(DF[1] == 0.0);

  // Open tests, laminar
  control = 0.5;
  opening.calculate(true, dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == Approx(.03 / 0.0000181625));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.003 / 0.0000181625));
  CHECK(DF[1] == 0.0);

  opening.calculate(true, -dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == Approx(-.03 / 0.0000181625));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.003 / 0.0000181625));
  CHECK(DF[1] == 0.0);

  // Open tests, turbulent, one way
  state1.density = 1.1041;
  state1.sqrt_density = std::sqrt(1.1041);

  double C = 0.125 * 1.414213562373095;
  double rho_diff = 0.1;
  double g_rho_diff = 0.98;
  double y = dp / g_rho_diff;
  double df0 = C * std::sqrt(dp) / g_rho_diff;
  double f0 = 2.0 * C * std::sqrt(dp) * y/3.0;
  double dfh = C * std::sqrt(std::abs((1.0 - y) / g_rho_diff));
  double fh = 2.0 * dfh * std::abs(g_rho_diff * (1.0 - y))/3.0;
  
  opening.calculate(false, dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == Approx(state0.sqrt_density * std::abs(fh - f0)));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(state0.sqrt_density * std::abs(dfh - df0)));
  CHECK(DF[1] == 0.0);

  y = -dp / g_rho_diff;
  dfh = C * std::sqrt(std::abs((1.0 - y) / g_rho_diff));
  fh = 2.0 * dfh * std::abs(g_rho_diff * (1.0 - y)) / 3.0;
  opening.calculate(false, -dp, multiplier, control, state0, state1, F, DF);
  CHECK(F[0] == Approx(-state1.sqrt_density * std::abs(fh - f0)));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(state1.sqrt_density * std::abs(dfh - df0)));
  CHECK(DF[1] == 0.0);

  // Open tests, turbulent, two way
  dp = 0.1;
  state1.density = 1.1041;
  state1.sqrt_density = std::sqrt(1.1041);

  C = 0.125 * 1.414213562373095;
  rho_diff = 0.1;
  g_rho_diff = 0.98;
  y = dp / g_rho_diff;
  df0 = C * std::sqrt(dp) / g_rho_diff;
  f0 = 2.0 * C * std::sqrt(dp) * y / 3.0;
  dfh = C * std::sqrt(std::abs((1.0 - y) / g_rho_diff));
  fh = 2.0 * dfh * std::abs(g_rho_diff * (1.0 - y)) / 3.0;

  int nf = opening.calculate(false, dp, multiplier, control, state0, state1, F, DF);
  CHECK(nf == 2);
  CHECK(F[0] == Approx(-state1.sqrt_density * fh));
  CHECK(F[1] == Approx(state0.sqrt_density * f0)); 
  CHECK(DF[0] == Approx(state1.sqrt_density* dfh));
  CHECK(DF[1] == Approx(state0.sqrt_density * df0));


  state0.density = 1.1041;
  state0.sqrt_density = std::sqrt(1.1041);
  state1.density = 1.2041;
  state1.sqrt_density = std::sqrt(1.2041);

  C = 0.125 * 1.414213562373095;
  rho_diff = 0.1;
  g_rho_diff = 0.98;
  y = dp / g_rho_diff;
  df0 = C * std::sqrt(dp) / g_rho_diff;
  f0 = 2.0 * C * std::sqrt(dp) * y / 3.0;
  dfh = C * std::sqrt(std::abs((1.0 - y) / g_rho_diff));
  fh = 2.0 * dfh * std::abs(g_rho_diff * (1.0 - y)) / 3.0;

  nf = opening.calculate(false, -dp, multiplier, control, state0, state1, F, DF);
  CHECK(nf == 2);
  CHECK(F[0] == Approx(state0.sqrt_density * fh));
  CHECK(F[1] == Approx(-state1.sqrt_density * f0));
  CHECK(DF[0] == Approx(state0.sqrt_density * dfh));
  CHECK(DF[1] == Approx(state1.sqrt_density * df0));

}
