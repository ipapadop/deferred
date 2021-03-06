/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("constant from expression", "[constant-from-constant-expression]")
{
  auto i = 10;

  auto ex = deferred::constant(i) * 2;
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);

  auto c = deferred::constant(ex);
  static_assert(deferred::is_constant_expression_v<decltype(c)>);

  // constant() from constant expression forces evaluation
  static_assert(std::is_same_v<decltype(c), decltype(deferred::constant(i))>);

  CHECK(c() == 20);
}

TEST_CASE("constant from chained constant expressions",
          "[constant-from-chained-constant-expressions]")
{
  auto c1 = deferred::constant(1);
  static_assert(deferred::is_constant_expression_v<decltype(c1)>);

  auto ex1 = c1 + deferred::constant(2);
  static_assert(deferred::is_constant_expression_v<decltype(ex1)>);

  auto ex2 = ex1 + ex1;
  static_assert(deferred::is_constant_expression_v<decltype(ex2)>);

  auto ex3 = ex2 - ex2;
  static_assert(deferred::is_constant_expression_v<decltype(ex3)>);

  CHECK(ex3() == 0);
}

TEST_CASE("constant from expression with variable", "[constant-from-variable]")
{
  auto v  = deferred::variable(2);
  auto ex = deferred::constant(4) * v;
  auto c  = deferred::constant(ex);
  CHECK(c() == 8);
}
