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

TEST_CASE("variables", "[variables]")
{
  SECTION("empty variable")
  {
    auto v1 = deferred::variable<int>();
    CHECK(v1() == int{});

    v1 = 42;
    CHECK(v1() == 42);
  }

  SECTION("initialization")
  {
    auto v1 = deferred::variable(42);
    CHECK(v1() == 42);
  }

  SECTION("assignment")
  {
    auto v1 = deferred::variable<int>();
    v1 = 42;
    CHECK(v1() == 42);
  }

  SECTION("expression")
  {
    auto v1 = deferred::variable<int>();
    auto e1 = v1 + deferred::constant(10);
    v1 = 42;
    CHECK(e1() == 52);
  }

#if 0
  // this should never work
  SECTION("incorrect expression")
  {
    auto e1 = deferred::variable<int>() + deferred::constant(10);
    CHECK(e1() == 10);
  }
#endif
}