// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("visitor traverses an expression composed through the public API", "[visitor]")
{
  auto variable   = deferred::variable(1);
  auto expression = deferred::if_(variable > 0).then_(variable + 1).else_(0);
  std::vector<std::size_t> nesting_levels;

  expression.visit([&](auto const&, std::size_t nesting) { nesting_levels.push_back(nesting); }, 7);

  CHECK(nesting_levels == std::vector<std::size_t>{7, 8, 9, 10, 10, 9, 10, 10, 8});
}

TEST_CASE("type_name is available from the umbrella header", "[type-name]")
{
  // This file includes only deferred/deferred.hpp.
  auto ex = deferred::constant(1) + deferred::constant(2);

  CHECK(deferred::type_name<int>() == "int");
  CHECK(deferred::type_name(ex).find("expression_") != std::string::npos);
}
