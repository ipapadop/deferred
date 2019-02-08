/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("simple expressions", "[expressions-simple]")
{
  auto i = 41;
  auto j = 3;

  SECTION("multiple expressions")
  {
    auto c1 = deferred::constant(i);
    auto ex1 = c1 + deferred::constant(j);
    auto ex2 = ex1 + ex1;
    auto ex3 = ex2 - ex2;
    CHECK(ex3() == 0);
  }

  SECTION("prvalue constants")
  {
    auto ex = deferred::constant(i) + deferred::constant(j);
    CHECK(ex() == i + j);
  }

  SECTION("literals")
  {
    auto ex = deferred::constant(i) * j;
    CHECK(ex() == i * j);
  }
}

TEST_CASE("constexpr expressions", "[expressions-constexpr]")
{
  SECTION("operations on constants")
  {
    constexpr auto ex = deferred::constant(5) * deferred::constant(4);
    static_assert(ex() == 5 * 4, "could not create constexpr");
    CHECK(ex() == 5 * 4);
  }
}

TEST_CASE("expressions with variables", "[expression-variable]")
{
  SECTION("variable + constant")
  {
    auto v = deferred::variable<int>();
    auto ex = v + deferred::constant(10);
    v = 42;
    CHECK(ex() == 52);
  }

  SECTION("variable + variable")
  {
    auto v1 = deferred::variable<int>();
    auto v2 = deferred::variable<double>();
    auto i = 10;
    auto d = 23.3;

    auto ex = v1 + v2;
    v1 = i;
    v2 = d;

    CHECK(ex() == i + d);
  }
}
