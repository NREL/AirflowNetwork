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
#include "catch.hpp"
#include "node.hpp"
#include "link.hpp"
#include "properties.hpp"
#include "powerlaw.hpp"
#include "eigen_transport.hpp"
#include "Eigen/Sparse"

TEST_CASE("Test a very simple network, explicit, Eigen", "[eigen_transport]")
{
  airflownetwork::Node<size_t, airflownetwork::properties::Fixed> node0("Node0");
  airflownetwork::Node<size_t, airflownetwork::properties::Fixed> node1("Node1");
  airflownetwork::PowerLaw<airflownetwork::properties::Fixed> powerlaw("powerlaw", 0.001, 0.001);
  std::vector<airflownetwork::Link<size_t, airflownetwork::properties::Fixed>> links;
  links.emplace_back("Link", node0, node1, powerlaw);

  airflownetwork::Filter filter(0.5);

  links[0].filters = std::vector<std::vector<airflownetwork::Filter>>(1);
  links[0].filters[0].push_back(filter);
  links[0].flow = 1.0;
  links[0].nf = 1;

  // Set up the indices
  node0.index = 0;
  node1.index = 1;

  // Set up the matrix
  Eigen::SparseMatrix<double> matrix(2,2);
  matrix.insert(0, 0) = matrix.insert(1, 0) = 0.0;

  // Fill in the matrix
  airflownetwork::transport::matrix<std::vector<airflownetwork::Link<size_t, airflownetwork::properties::Fixed>>, 
    Eigen::SparseMatrix<double>, size_t>(0, matrix, links);

  CHECK(matrix.nonZeros() == 2);
  CHECK(matrix.coeff(0, 0) == -1.0);
  CHECK(matrix.coeff(1, 0) ==  0.5);

  // Compute one step
  Eigen::VectorXd g(2);
  g << 0.0, 0.0;
  Eigen::VectorXd r(2);
  r << 0.0, 0.0;
  Eigen::VectorXd c(2);
  c << 0.5, 0.5;
  Eigen::VectorXd c0(2);
  c0 << 0.5, 0.5;

  Eigen::VectorXd a0(2);
  a0 << 1.0, 1.0;
  Eigen::VectorXd a(2);
  a << 1.0, 1.0;

  airflownetwork::transport::explicit_euler<Eigen::SparseMatrix<double>, Eigen::VectorXd>(0.25, matrix, g, r, a0, a, c);
  CHECK(c(0) == 0.5 - 0.25 * 0.5);
  CHECK(c(1) == 0.5 + 0.25 * 0.5 * 0.5);
  // Check for mass conservation
  CHECK(0.5 * (c0(0) - c(0)) == c(1) - c0(1));
}

TEST_CASE("Test a very simple network, opposite direction, explicit, eigen", "[eigen_transport]")
{
  airflownetwork::Node<size_t, airflownetwork::properties::Fixed> node0("Node0");
  airflownetwork::Node<size_t, airflownetwork::properties::Fixed> node1("Node1");
  airflownetwork::PowerLaw<airflownetwork::properties::Fixed> powerlaw("powerlaw", 0.001, 0.001);
  std::vector<airflownetwork::Link<size_t, airflownetwork::properties::Fixed>> links;
  links.emplace_back("Link", node0, node1, powerlaw);

  airflownetwork::Filter filter(0.5);

  links[0].filters = std::vector<std::vector<airflownetwork::Filter>>(1);
  links[0].filters[0].push_back(filter);
  links[0].flow = -1.0;
  links[0].nf = 1;

  // Set up the indices
  node0.index = 0;
  node1.index = 1;

  // Set up the matrix
  Eigen::SparseMatrix<double> matrix(2, 2);
  matrix.insert(0, 1) = matrix.insert(1, 1) = 0.0;

  // Fill in the matrix
  airflownetwork::transport::matrix<std::vector<airflownetwork::Link<size_t, airflownetwork::properties::Fixed>>,
    Eigen::SparseMatrix<double>, size_t>(0, matrix, links);

  CHECK(matrix.nonZeros() == 2);
  CHECK(matrix.coeff(1, 1) == -1.0);
  CHECK(matrix.coeff(0, 1) == 0.5);

  // Compute one step
  Eigen::VectorXd g(2);
  g << 0.0, 0.0;
  Eigen::VectorXd r(2);
  r << 0.0, 0.0;
  Eigen::VectorXd c(2);
  c << 0.5, 0.5;
  Eigen::VectorXd c0(2);
  c0 << 0.5, 0.5;
  Eigen::VectorXd a0(2);
  a0 << 1.0, 1.0;
  Eigen::VectorXd a(2);
  a << 1.0, 1.0;

  airflownetwork::transport::explicit_euler<Eigen::SparseMatrix<double>, Eigen::VectorXd>(0.25, matrix, g, r, a0, a, c);
  CHECK(c(1) == 0.5 - 0.25 * 0.5);
  CHECK(c(0) == 0.5 + 0.25 * 0.5 * 0.5);
  // Check for mass conservation
  CHECK(0.5 * (c0(1) - c(1)) == c(0) - c0(0));
}

TEST_CASE("Test a very simple network with zero, explicit, Eigen", "[eigen_transport]")
{
  airflownetwork::Node<size_t, airflownetwork::properties::Fixed> node0("Node0");
  airflownetwork::Node<size_t, airflownetwork::properties::Fixed> node1("Node1");
  airflownetwork::PowerLaw<airflownetwork::properties::Fixed> powerlaw("powerlaw", 0.001, 0.001);
  std::vector<airflownetwork::Link<size_t, airflownetwork::properties::Fixed>> links;
  links.emplace_back("Link", node0, node1, powerlaw);

  airflownetwork::Filter filter(0.5);

  links[0].filters = std::vector<std::vector<airflownetwork::Filter>>(1);
  links[0].filters[0].push_back(filter);
  links[0].flow = 1.0;
  links[0].nf = 1;

  // Set up the indices
  node0.index = 0;
  node1.index = 1;

  // Set up the matrix
  Eigen::SparseMatrix<double> matrix(2, 2);
  matrix.insert(0, 0) = matrix.insert(1, 0) = 0.0;

  // Set up the concentrations
  Eigen::VectorXd c(2);
  c << 0.0, 0.5;
  Eigen::VectorXd c0(2);
  c0 << 0.0, 0.5;
  Eigen::VectorXd a0(2);
  a0 << 1.0, 1.0;
  Eigen::VectorXd a(2);
  a << 1.0, 1.0;

  // Fill in the matrix
  airflownetwork::transport::matrix<std::vector<airflownetwork::Link<size_t, airflownetwork::properties::Fixed>>,
    Eigen::SparseMatrix<double>, size_t>(0, matrix, links);

  CHECK(matrix.nonZeros() == 2);
  CHECK(matrix.coeff(0, 0) == -1.0);
  CHECK(matrix.coeff(1, 0) == 0.5);

  // Compute one step
  Eigen::VectorXd g(2);
  g << 0.0, 0.0;
  Eigen::VectorXd r(2);
  r << 0.0, 0.0;

  airflownetwork::transport::explicit_euler<Eigen::SparseMatrix<double>, Eigen::VectorXd>(0.25, matrix, g, r, a0, a, c);
  CHECK(c(0) == c0(0));
  CHECK(c(1) == c0(1));
}

TEST_CASE("Test a very simple network, implicit, Eigen", "[eigen_transport]")
{
  airflownetwork::Node<size_t, airflownetwork::properties::Fixed> node0("Node0");
  airflownetwork::Node<size_t, airflownetwork::properties::Fixed> node1("Node1");
  airflownetwork::PowerLaw<airflownetwork::properties::Fixed> powerlaw("powerlaw", 0.001, 0.001);
  std::vector<airflownetwork::Link<size_t, airflownetwork::properties::Fixed>> links;
  links.emplace_back("Link", node0, node1, powerlaw);

  airflownetwork::Filter filter(0.5);

  links[0].filters = std::vector<std::vector<airflownetwork::Filter>>(1);
  links[0].filters[0].push_back(filter);
  links[0].flow = 1.0;
  links[0].nf = 1;

  // Set up the indices
  node0.index = 0;
  node1.index = 1;

  // Set up the matrix
  Eigen::SparseMatrix<double> matrix(2, 2);
  matrix.insert(0, 0) = matrix.insert(1, 0) = 0.0;

  // Fill in the matrix
  airflownetwork::transport::matrix<std::vector<airflownetwork::Link<size_t, airflownetwork::properties::Fixed>>,
    Eigen::SparseMatrix<double>, size_t>(0, matrix, links);

  CHECK(matrix.nonZeros() == 2);
  CHECK(matrix.coeff(0, 0) == -1.0);
  CHECK(matrix.coeff(1, 0) == 0.5);

  matrix.insert(1, 1) = 0.0;

  // Compute one step
  Eigen::VectorXd g(2);
  g << 0.0, 0.0;
  Eigen::VectorXd r(2);
  r << 0.0, 0.0;
  Eigen::VectorXd c(2);
  c << 0.5, 0.5;
  Eigen::VectorXd c0(2);
  c0 << 0.5, 0.5;
  Eigen::VectorXd a0(2);
  a0 << 1.0, 1.0;
  Eigen::VectorXd a(2);
  a << 1.0, 1.0;

  Eigen::BiCGSTAB<Eigen::SparseMatrix<double>> solver;

  airflownetwork::transport::implicit_euler(solver, 0.25, matrix, g, r, a0, a, c);
  CHECK(c(0) == Approx(0.4));
  CHECK(c(1) == Approx(0.55));
  // Check for mass conservation
  CHECK(0.5 * (c0(0) - c(0)) == Approx(c(1) - c0(1)));
}

