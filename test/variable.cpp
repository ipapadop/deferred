/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/operators.hpp"
#include "deferred/variable.hpp"

TEST_CASE("empty variable", "[variable-empty]")
{
  auto v = deferred::variable<int>();
  CHECK(v() == int{});

  SECTION("assignment")
  {
    v = 42;
    CHECK(v() == 42);
  }
}

TEST_CASE("initialized variable", "[variable-init]")
{
  auto v = deferred::variable(42);
  CHECK(v() == 42);
}
