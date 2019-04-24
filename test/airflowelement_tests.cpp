#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "powerlaw.hpp"

TEST_CASE("Test the power law element", "[PowerLaw]")
{
  airflownetwork::PowerLaw<airflownetwork::properties::Fixed> powerlaw("powerlaw", 0.001, 0.001);
  airflownetwork::State<airflownetwork::properties::Fixed> state0;
  airflownetwork::State<airflownetwork::properties::Fixed> state1;

  std::array<double, 2> F{ {0.0, 0.0} };
  std::array<double, 2> DF{ {0.0, 0.0} };

  double dp{ 10.0 };

  // Linearized test
  double C = powerlaw.linearize(1.0, state0, state1);
  CHECK(C == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));

  // Laminar tests
  powerlaw.calculate(true, dp, 1.0, state0, state1, F, DF);
  CHECK(F[0] == .01 * std::sqrt(1.2041) / 0.0000181625);
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));
  CHECK(DF[1] == 0.0);

  powerlaw.calculate(true, -dp, 1.0, state0, state1, F, DF);
  CHECK(F[0] == -.01 * std::sqrt(1.2041) / 0.0000181625);
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.001 * std::sqrt(1.2041) / 0.0000181625));
  CHECK(DF[1] == 0.0);

  // Turbulent tests
  powerlaw.calculate(false, dp, 1.0, state0, state1, F, DF);
  CHECK(F[0] == .001 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000065 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);

  powerlaw.calculate(false, -dp, 1.0, state0, state1, F, DF);
  CHECK(F[0] == -.001 * std::pow(10.0, 0.65));
  CHECK(F[1] == 0.0);
  CHECK(DF[0] == Approx(.000065 * std::pow(10.0, 0.65)));
  CHECK(DF[1] == 0.0);
}
