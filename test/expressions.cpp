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
    auto e1 = c1 + deferred::constant(j);
    auto e2 = e1 + e1;
    auto e3 = e2 - e2;
    CHECK(e3() == 0);
  }

  SECTION("prvalue constants")
  {
    auto e1 = deferred::constant(i) + deferred::constant(j);
    CHECK(e1() == i + j);
  }

  SECTION("literals")
  {
    auto e1 = deferred::constant(i) * j;
    CHECK(e1() == i * j);
  }
}

TEST_CASE("constexpr expressions", "[expressions-constexpr]")
{
  SECTION("operations on constants")
  {
    constexpr auto e1 = deferred::constant(5) * deferred::constant(4);
    static_assert(e1() == 5 * 4, "could not create constexpr");
    CHECK(e1() == 5 * 4);
  }
}

TEST_CASE("expressions with variables", "[expression-variable]")
{
  SECTION("variable + constant")
  {
    auto v1 = deferred::variable<int>();
    auto e1 = v1 + deferred::constant(10);
    v1 = 42;
    CHECK(e1() == 52);
  }

  SECTION("variable + variable")
  {
    auto v1 = deferred::variable<int>();
    auto v2 = deferred::variable<double>();
    auto i = 10;
    auto d = 23.3;

    auto e1 = v1 + v2;
    v1 = i;
    v2 = d;

    CHECK(e1() == i + d);
  }
}
