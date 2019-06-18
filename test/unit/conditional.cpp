/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/conditional.hpp"

TEST_CASE("conditional with literal", "[conditional-literal]")
{
  auto ex = deferred::if_then_else(true, 42, 10);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 42);
}

TEST_CASE("conditional with different result types", "[conditional-diff-types]")
{
  auto ex = deferred::if_then_else(true, 42, false);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 42);
}

TEST_CASE("conditional with lambda", "[conditional-lambdas]")
{
  auto i  = 0;
  auto j  = 0;
  auto k  = 0;
  auto ex = deferred::if_then_else(
    [&] {
      ++i;
      return true;
    },
    [&] {
      ++j;
      return 42;
    },
    [&] {
      ++k;
      return 10;
    });

  // it cannot detect functions with side-effects
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
  CHECK(ex() == 42);
  CHECK(i == 1);
  CHECK(j == 1);
  CHECK(k == 0);
}
