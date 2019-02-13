/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("constant from expression",
          "[constant-from-constant-expression]")
{
  auto i = 10;
  CHECK(deferred::is_constant_expression_v<decltype(i)>);

  auto ex = deferred::constant(i) * 2;
  CHECK(deferred::is_constant_expression_v<decltype(ex)>);

  auto c = deferred::constant(ex);
  CHECK(deferred::is_constant_expression_v<decltype(c)>);

  CHECK(c() == 20);
}

TEST_CASE("constant from chained constant expressions",
          "[constant-from-chained-constant-expressions]")
{
  auto c1 = deferred::constant(1);
  CHECK(deferred::is_constant_expression_v<std::decay_t<decltype(c1)>>);

  auto ex1 = c1 + deferred::constant(2);
  CHECK(deferred::is_constant_expression_v<std::decay_t<decltype(ex1)>>);

  auto ex2 = ex1 + ex1;
  CHECK(deferred::is_constant_expression_v<std::decay_t<decltype(ex2)>>);

  auto ex3 = ex2 - ex2;
  CHECK(deferred::is_constant_expression_v<std::decay_t<decltype(ex3)>>);

  CHECK(ex3() == 0);
}

#if 0
// cannot create constant from expression with variable
TEST_CASE("constant from expression with variable",
          "[constant-from-non-constant-expression]")
{
  auto v  = deferred::variable(2);
  auto ex = deferred::constant(4) * v;
  auto c  = deferred::constant(ex);
  CHECK(c() == 8);
  FAIL("Cannot create constant from expression with a variable");
}
#endif
