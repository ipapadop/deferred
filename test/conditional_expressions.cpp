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
  auto ex = deferred::if_then_else(deferred::constant(true), deferred::constant(42), deferred::constant(10));
  CHECK(ex() == 42);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);

  auto c = deferred::constant(ex);
  CHECK(c() == 42);
}

#if 0

TEST_CASE("conditional with variable", "[conditional-variable]")
{
  auto v = deferred::variable<bool>();
  auto ex = deferred::if_then_else(v, deferred::constant(42), deferred::constant(10));
  v = true;
  CHECK(ex() == 42);

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);

  auto c = deferred::constant(ex);
  CHECK(c() == 42);
}

#endif
