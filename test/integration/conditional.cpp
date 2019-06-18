/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("conditional with constants", "[conditional-constants]")
{
  auto ex = deferred::if_then_else(
    deferred::constant(true), deferred::constant(42), deferred::constant(10));

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 42);

  auto c = deferred::constant(ex);
  CHECK(c() == 42);
}

TEST_CASE("conditional with variable", "[conditional-variable]")
{
  auto v = deferred::variable<bool>();
  auto ex =
    deferred::if_then_else(v, deferred::constant(42), deferred::constant(10));
  v = true;

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 42);
}

TEST_CASE("conditional chain", "[conditional-chain]")
{
  auto v   = deferred::variable<bool>();
  auto ex1 = deferred::if_then_else(v, 42, 10);
  auto ex2 = deferred::if_then_else(ex1 == 42, 1337, 0);

  static_assert(!deferred::is_constant_expression_v<decltype(ex2)>);

  v = true;
  CHECK(ex2() == 1337);

  v = false;
  CHECK(ex2() == 0);
}

TEST_CASE("conditional with nested deferred", "[conditional-deferred]")
{
  auto v   = deferred::variable<bool>();
  auto ex1 = deferred::if_then_else(v, 42, 10);
  auto ex2 = deferred::if_then_else([&] { return ex1 == 42; }, 1337, 0);

  static_assert(!deferred::is_constant_expression_v<decltype(ex2)>);

  v = true;
  CHECK(ex2() == 1337);

  v = false;
  CHECK(ex2() == 0);
}
