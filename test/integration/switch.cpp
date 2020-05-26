/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <cstring>

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

namespace {

int foo()
{
  return 10;
}

} // namespace

TEST_CASE("switch with constants", "[switch-constants]")
{
  auto c = deferred::constant(10);
  auto ex =
    deferred::switch_(c,
                      deferred::default_(std::string("unknown")),
                      deferred::case_(10, [] { return std::string("10"); }));

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == "10");

  auto c2 = deferred::constant(ex);
  CHECK(c2() == "10");
}

TEST_CASE("switch with c-strings", "[switch-c-strings]")
{
  auto var = deferred::variable<int>();
  auto ex  = deferred::switch_(var,
                              deferred::default_("unknown"),
                              deferred::case_(10, [] { return "10"; }),
                              deferred::case_(12, [] { return "12"; }));

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  var = 10;
  CHECK(std::strcmp(ex(), "10") == 0);

  var = 11;
  CHECK(std::strcmp(ex(), "unknown") == 0);
}

TEST_CASE("switch checking against function", "[switch-function]")
{
  auto c  = deferred::constant(foo());
  auto ex = deferred::switch_(
    c,
    deferred::default_(std::string("unknown")),
    deferred::case_([] { return foo(); }, [] { return std::string("foo"); }));

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == "foo");
}

TEST_CASE("switch with expressions", "[switch-expressions]")
{
  auto condition = deferred::variable<int>();
  auto label     = deferred::variable<int>();
  auto ex        = deferred::switch_(condition,
                              deferred::default_(condition + 1),
                              deferred::case_(label, condition + 2));

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);

  condition = 1;
  label     = 1;
  CHECK(ex() == 3);

  condition = 1;
  label     = 5;
  CHECK(ex() == 2);
}
