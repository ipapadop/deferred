/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/switch.hpp"

TEST_CASE("default with literal", "[default-literal]")
{
  auto ex = deferred::default_(2);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 2);
}

TEST_CASE("default with lambda", "[default-lambda]")
{
  auto ex = deferred::default_([] { return 2; });
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 2);
}

TEST_CASE("case with literal", "[case-literal]")
{
  auto ex = deferred::case_(2, 0);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex.compare(1) == false);
  CHECK(ex.compare(2) == true);
  CHECK(ex() == 0);
}

TEST_CASE("case with lambda", "[case-lambda]")
{
  auto ex = deferred::case_([] { return 2; }, [] { return 0; });
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex.compare(1) == false);
  CHECK(ex.compare(2) == true);
  CHECK(ex() == 0);
}

TEST_CASE("switch with two literals", "[switch-two-literals]")
{
  auto ex = deferred::switch_(2,
                              deferred::default_(100),
                              deferred::case_(1, [] { return 0; }),
                              deferred::case_(2, [] { return 10; }));
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 10);
}

TEST_CASE("switch with three literals", "[switch-three-literals]")
{
  auto ex = deferred::switch_(3,
                              deferred::default_(1000),
                              deferred::case_(1, [] { return 0; }),
                              deferred::case_(2, [] { return 10; }),
                              deferred::case_(3, [] { return 100; }));
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 100);
}

TEST_CASE("switch with two literals defaulting",
          "[switch-two-literals-default]")
{
  auto ex = deferred::switch_(10,
                              deferred::default_(100),
                              deferred::case_(1, [] { return 0; }),
                              deferred::case_(2, [] { return 10; }));
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 100);
}

TEST_CASE("switch with three literals defaulting",
          "[switch-three-literals-default]")
{
  auto ex = deferred::switch_(0,
                              deferred::default_(1000),
                              deferred::case_(1, [] { return 0; }),
                              deferred::case_(2, [] { return 10; }),
                              deferred::case_(3, [] { return 100; }));
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 1000);
}
