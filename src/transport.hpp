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

template <typename T> class HasCoeffRef
{
private:
  typedef char YesType[1];
  typedef char NoType[2];

  template <typename C> static YesType& test(decltype(&C::coeffRef));
  template <typename C> static NoType& test(...);

public:
  enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};
/*
template <typename T, typename = int>
struct HasCoeffRef : std::false_type {};

template <typename T>
struct HasCoeffRef <T, decltype((void)T::coeffRef, 0)> : std::true_type {};
*/
template <typename L, typename M, typename K> void matrix(const K& key, typename std::enable_if<HasCoeffRef<M>::value, M>::type& matrix, L& links)
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
        matrix(link.node0.index) -= link.flow; // This is a diagonal entry
        matrix(link.index1) += link.flow * ineff;
      } else if (link.flow < 0) {
        matrix(link.index0) -= link.flow * ineff;
        matrix(link.node1.index) += link.flow; // This is a diagonal entry
      }
    } else if (link.nf == 2) {
      // TODO!!!
    }
  inactive: ;
  }
}

/*
template <typename T> class HasArray
{
private:
  typedef char YesType[1];
  typedef char NoType[2];

  template <typename C> static YesType& test(decltype(&C::array));
  template <typename C> static NoType& test(...);

public:
  enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
};
*/

template <typename T, typename = int>
struct HasCWiseProduct : std::false_type {};

template <typename T>
struct HasCWiseProduct <T, decltype((void)T::cwiseProduct, 0)> : std::true_type {};

//template <typename M, typename V>
//void explicit_euler_transport(double dt, M &matrix, typename std::enable_if<HasCWiseProduct<V>::value, V>::type &G, V &R, V &C)
//{
//  C = dt * matrix * C + dt * G + C - dt * R.cwiseProduct(C);
//}

template <typename M, typename V> void explicit_euler(double dt, M& matrix, V& G, V& R, V& C)
{
  C = dt * matrix * C + dt * G + C - dt * R.cwiseProduct(C);
}

//template <typename M, typename V> void explicit_euler_transport(double dt, M& matrix, V& G, V& R, V& C)
//{
//  for (decltype(G.size()) i = 0; i < G.size(); ++i) {
//    G[i] = dt * G[i] + (1.0 - dt * R[i]) * C[i];
//  }
//  C = dt * matrix * C + G;
//}

template <typename S, typename M, typename V> void implicit_euler(S &solver, double dt, M& matrix, V& G, V& R, V& C0, V&C)
{
  // RHS
  //C0 += dt * G;
  //bool halfstep{ false };
  // LHS
  //auto diagonal = V::Ones(R.size()) - dt * R - dt * matrix.diagonal();
  //for (auto &val : diagonal) {
  //  if (val == 0) {
  //    halfstep = true;
  //    break;
  //  }
  //}
  matrix *= -dt;
  matrix += (V::Ones(R.size()) - dt*R).asDiagonal(); //(C - (dt * R.array() * C.array())).asDiagonal(); // C.asDiagonal() * (1.0 - dt * R);
  //std::cout << matrix.coeff(0, 0) << ' ' << matrix.coeff(0, 1) << ' ' << matrix.coeff(1, 0) << ' ' << matrix.coeff(1, 1) << ' ' << std::endl;
  //std::cout << C0[0] << ' ' << C0[1] << std::endl;
  solver.compute(matrix);
  // Solve
  C = solver.solve(C0 + dt*G);
  //std::cout << "#iterations:     " << solver.iterations() << std::endl;
  //std::cout << "estimated error: " << solver.error() << std::endl;
  //std::cout << C0[0] << ' ' << C0[1] << std::endl;
  //std::cout << C[0] << ' ' << C[1] << std::endl;

}

/*
template <typename S, typename M, typename V> void implicit_euler_transport(double dt, M& matrix, V& G, V& R, V& C)
{
  // RHS
  for (decltype(G.size()) i = 0; i < G.size(); ++i) {
    C[i] += dt * G[i];
  }
  // LHS
  C = dt * matrix * C + G;
}
*/
}
}

#endif // !AIRFLOWNETWORK_TRANSPORT_HPP
