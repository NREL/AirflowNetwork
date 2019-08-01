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
#ifndef AIRFLOWNETWORK_TRANSPORT_HPP
#define AIRFLOWNETWORK_TRANSPORT_HPP

#include <string>
#include <vector>
#include <type_traits>
#include <iostream>
#include "filters.hpp"

namespace airflownetwork {
namespace transport {

template <typename L, typename M, typename K> void matrix(const K& key, M& matrix, L& links)
{
  for (auto& link : links) {
    // Compute the inefficiency
    double ineff = 1.0;
    for (auto& filter : link.filters[key]) {
      // Expect filters found via a key, get the combined efficiency with a schedule value
      double eff = filter.efficiency * filter.control;
      if (eff == 1.0) {
        // Nothing is going to be transported
        // link.active = false;
        goto inactive;
      }
      ineff *= (1.0 - eff);
    }
    if (link.nf == 1) {
      if (link.flow > 0.0) {
        matrix.coeffRef(link.node0.index, link.node0.index) -= link.flow; // This is a diagonal entry
        matrix.coeffRef(link.node1.index, link.node0.index) += link.flow * ineff;
      } else if (link.flow < 0) {
        matrix.coeffRef(link.node0.index, link.node1.index) -= link.flow * ineff;
        matrix.coeffRef(link.node1.index, link.node1.index) += link.flow; // This is a diagonal entry
      }
    } else if (link.nf == 2) { // This is not tested yet
      if (link.flow0 > 0.0) {
        matrix.coeffRef(link.node0.index, link.node0.index) -= link.flow0; // This is a diagonal entry
        matrix.coeffRef(link.node1.index, link.node0.index) += link.flow0 * ineff;
      }
      if (link.flow1 > 0.0) {
        matrix.coeffRef(link.node0.index, link.node1.index) += link.flow1 * ineff;
        matrix.coeffRef(link.node1.index, link.node1.index) -= link.flow1; // This is a diagonal entry
      }
    }
  inactive:;
  }
}

template <typename M, typename V> void explicit_euler(double h, M& matrix, V& G0, V& R0, V& A0, V& A, V& C)
{
  C = (A0.cwiseProduct(C) + h *(matrix*C - R0.cwiseProduct(C) + G0)).cwiseQuotient(A);
}

template <typename S, typename M, typename V> void implicit_euler(S &solver, double h, M& matrix, V& G, V& R, V& A0, V& A, V& C)
{
  // Set up
  matrix *= -h;
  matrix += (A + h*R).asDiagonal();
  solver.compute(matrix);
  // Solve
  G *= h;
  G += A.cwiseProduct(C);
  C = solver.solve(G);
}

template <typename S, typename M, typename V> void crank_nicolson(S& solver, double h, M& matrix, V& G0, V& G, V& R0, V& R, V& A0, V& A, V& C)
{
  h *= 0.5;
  V hH = h * (matrix * C - R0.cwiseProduct(C) + G0);
  // Set up
  matrix *= -h;
  matrix += (A + h * R).asDiagonal();
  solver.compute(matrix);
  // Solve
  G *= h;
  G += A0.cwiseProduct(C);
  G += hH;
  C = solver.solve(G);
}

}

}

#endif // !AIRFLOWNETWORK_TRANSPORT_HPP
