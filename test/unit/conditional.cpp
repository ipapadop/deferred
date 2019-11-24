/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/conditional.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"

TEST_CASE("conditional with literal", "[conditional-literal]")
{
  auto ex = deferred::if_then_else(true, 42, 10);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 42);
}

TEST_CASE("conditional with constexpr", "[conditional-constexpr]")
{
  constexpr auto ex = deferred::if_then_else(true, 42, 10);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(ex() == 42, "constexpr failed");
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
  auto ex = deferred::if_then_else([&] { return i++ == 1; },
                                   [&] { return ++j; },
                                   [&] { return k += 2; });

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
  CHECK(ex() == 2);
  CHECK(i == 1);
  CHECK(j == 0);
  CHECK(k == 2);
}

TEST_CASE("conditional with mutable lambda", "[conditional-mutable-lambdas]")
{
  auto i  = 0;
  auto j  = 0;
  auto k  = 0;
  auto ex = deferred::if_then_else([i]() mutable { return i++ == 1; },
                                   [j]() mutable { return ++j; },
                                   [k]() mutable { return k += 2; });

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
  CHECK(ex() == 2);
  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
}
