/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/switch.hpp"

TEST_CASE("switch with literal", "[switch-literal]")
{
  auto ex = deferred::switch_(2,
                              deferred::case_(1, [] { return 0; }),
                              deferred::case_(2, [] { return 10; }));

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 10);
}

TEST_CASE("switch with literal and default", "[switch-literal-default]")
{
  auto ex = deferred::switch_(3,
                              deferred::case_(1, [] { return 0; }),
                              deferred::case_(2, [] { return 10; }),
                              deferred::default_([] { return 100; }));

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 100);
}

TEST_CASE("switch with case constants", "[switch-case-constant]")
{
  auto ex = deferred::switch_(3, 0, 10, 100);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 100);
}
