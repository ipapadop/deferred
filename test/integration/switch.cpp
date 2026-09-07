// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

namespace {

int foo()
{
  return 10;
}

} // namespace

TEST_CASE("switch with constants", "[switch-constants]")
{
  auto c  = deferred::constant(10);
  auto ex = deferred::switch_(c)
              .case_(10)
              .then_([] { return std::string("10"); })
              .default_(std::string("unknown"));

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == "10");

  auto c2 = deferred::constant(ex);
  CHECK(c2() == "10");
}

TEST_CASE("switch with c-strings", "[switch-c-strings]")
{
  auto var = deferred::variable<int>();
  auto ex  = deferred::switch_(var)
               .case_(10)
               .then_([] { return "10"; })
               .case_(12)
               .then_([] { return "12"; })
               .default_("unknown");

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  var = 10;
  CHECK(std::strcmp(ex(), "10") == 0);

  var = 11;
  CHECK(std::strcmp(ex(), "unknown") == 0);
}

TEST_CASE("switch checking against function", "[switch-function]")
{
  auto c  = deferred::constant(foo());
  auto ex = deferred::switch_(c)
              .case_([] { return foo(); })
              .then_([] { return std::string("foo"); })
              .default_(std::string("unknown"));

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == "foo");
}

TEST_CASE("switch with expressions", "[switch-expressions]")
{
  auto condition = deferred::variable<int>();
  auto label     = deferred::variable<int>();
  auto ex = deferred::switch_(condition).case_(label).then_(condition + 2).default_(condition + 1);

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);

  condition = 1;
  label     = 1;
  CHECK(ex() == 3);

  condition = 1;
  label     = 5;
  CHECK(ex() == 2);
}

TEST_CASE("cases appended to switch", "[switch-append]")
{
  auto var           = deferred::variable<int>();
  auto ex            = deferred::switch_(var)
                         .case_(10)
                         .then_([] { return "10"; })
                         .case_(12)
                         .then_([] { return "12"; })
                         .default_("unknown");
  auto const& source = ex;
  auto expanded =
    source.case_(10).then_([] { return "new 10"; }).case_(11).then_([] { return "11"; });

  var = 10;
  CHECK(std::strcmp(expanded(), "10") == 0);

  var = 11;
  CHECK(std::strcmp(ex(), "unknown") == 0);
  CHECK(std::strcmp(expanded(), "11") == 0);

  var = 13;
  CHECK(std::strcmp(expanded(), "unknown") == 0);
}
