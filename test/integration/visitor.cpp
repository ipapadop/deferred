// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("visitor traverses an expression composed through the public API", "[visitor]")
{
  auto variable   = deferred::variable(1);
  auto expression = deferred::if_(variable > 0, variable + 1).else_(0);
  std::vector<std::size_t> nesting_levels;

  expression.visit([&](auto const&, std::size_t nesting) { nesting_levels.push_back(nesting); });

  CHECK(nesting_levels == std::vector<std::size_t>{0, 1, 2, 3, 3, 2, 3, 3, 1});
}
