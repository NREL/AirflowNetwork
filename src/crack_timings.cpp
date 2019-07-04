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
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include "properties.hpp"
#include "element.hpp"

int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "usage: crack_timings <number>" << std::endl;
    return 1;
  }

  std::array<double, 2> F{ {0.0, 0.0} };
  std::array<double, 2> DF{ {0.0, 0.0} };

  int count = std::stoi(argv[1]);

  std::vector<double> pdrop(count);
  std::vector<double> flow(count);
  std::vector<double> flow0(count);
  std::vector<airflownetwork::State<airflownetwork::properties::EnergyPlus>> M(count);
  std::vector<airflownetwork::State<airflownetwork::properties::EnergyPlus>> N(count);

  std::mt19937 rng;

  std::uniform_real_distribution<double> pressure_drop(-50.0, 50.0);
  std::uniform_real_distribution<double> pressure(101325.0-50.0, 101325+50.0);
  std::uniform_real_distribution<double> temperature(101325.0 - 50.0, 101325.0 + 50.0);

  // Set up the states
  for (size_t i = 0; i < M.size(); i++) {
    pdrop[i] = pressure_drop(rng);
    M[i].pressure = pressure(rng);
    M[i].temperature = temperature(rng);
    M[i].update();
    N[i].pressure = pressure(rng);
    N[i].temperature = temperature(rng);
    N[i].update();
  }

  // Calculate with genericCrack0
  auto start0 = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < M.size(); i++) {
    airflownetwork::genericCrack0(false, 0.0001, 0.65, pdrop[i], M[i], N[i], F, DF);
    flow0[i] = F[0];
  }
  auto stop0 = std::chrono::high_resolution_clock::now();

  // Calculate with genericCrack
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < M.size(); i++) {
    airflownetwork::genericCrack(false, 0.0001, 0.65, pdrop[i], M[i], N[i], F, DF);
    flow[i] = F[0];
  }
  auto stop = std::chrono::high_resolution_clock::now();

  
  auto duration0 = std::chrono::duration_cast<std::chrono::microseconds>(stop0 - start0);
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

  std::cout << "genericCrack0: " << duration0.count() << " microseconds" << std::endl;
  std::cout << " genericCrack: " << duration.count() << " microseconds" << std::endl;
  std::cout << double(duration0.count()- duration.count()) / double(duration0.count()) << std::endl;

  return 0;
}