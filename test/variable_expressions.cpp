/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("operation of variable with constant", "[variable-op-constant]")
{
  auto v  = deferred::variable<int>();
  auto ex = v + deferred::constant(10);
  v       = 42;

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 52);
}

TEST_CASE("operation of variable with variable", "[variable-op-variable]")
{
  auto v1 = deferred::variable<int>();
  auto v2 = deferred::variable<double>();
  auto i  = 10;
  auto d  = 23.3;

  auto ex = v1 + v2;
  v1      = i;
  v2      = d;

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == i + d);
}

TEST_CASE("variable from constant", "[variable-from-constant]")
{
  auto i = 10;
  auto c = deferred::constant(i);
  auto v = deferred::variable(c);

  static_assert(std::is_same_v<decltype(v), decltype(deferred::variable(10))>);
  CHECK(v() == i);
}

TEST_CASE("variable from constant expression",
          "[variable-from-constant-expression]")
{
  auto ex = deferred::constant(10) + deferred::constant(20);
  auto v  = deferred::variable(ex);

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  CHECK(v() == 30);
}
