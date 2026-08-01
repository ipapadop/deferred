// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "deferred/switch.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"

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

TEST_CASE("switch with two literals defaulting", "[switch-two-literals-default]")
{
  auto ex = deferred::switch_(10,
                              deferred::default_(100),
                              deferred::case_(1, [] { return 0; }),
                              deferred::case_(2, [] { return 10; }));
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 100);
}

TEST_CASE("switch with three literals defaulting", "[switch-three-literals-default]")
{
  auto ex = deferred::switch_(0,
                              deferred::default_(1000),
                              deferred::case_(1, [] { return 0; }),
                              deferred::case_(2, [] { return 10; }),
                              deferred::case_(3, [] { return 100; }));
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 1000);
}

TEST_CASE("switch with heterogeneous types", "[switch-variant]")
{
  auto ex = deferred::switch_(1, deferred::default_("unknown"), deferred::case_(1, 42));

  using result_type = decltype(ex());
  static_assert(std::is_same_v<result_type, std::variant<char const*, int>>);

  auto res = ex();
  CHECK(std::get<int>(res) == 42);

  auto ex2  = deferred::switch_(2, deferred::default_("unknown"), deferred::case_(1, 42));
  auto res2 = ex2();
  CHECK(std::get<char const*>(res2) == std::string("unknown"));
}

TEST_CASE("append case with heterogeneous type", "[switch-append-variant]")
{
  auto ex       = deferred::switch_(2, deferred::default_("unknown"), deferred::case_(1, 42));
  auto expanded = std::move(ex).append(deferred::case_(2, 2.5));

  using result_type = decltype(expanded());
  static_assert(std::is_same_v<result_type, std::variant<char const*, int, double>>);

  CHECK(std::get<double>(expanded()) == 2.5);
}

TEST_CASE("append case to switch with move-only body", "[switch-append-move-only]")
{
  auto ex =
    deferred::switch_(1,
                      deferred::default_(0),
                      deferred::case_(1, [value = std::make_unique<int>(42)] { return *value; }));
  auto expanded = std::move(ex).append(deferred::case_(2, 2));

  CHECK(expanded() == 42);
}

TEST_CASE("append case with void result", "[switch-append-void]")
{
  auto ex       = deferred::switch_(2, deferred::default_(0), deferred::case_(1, 1));
  auto expanded = std::move(ex).append(deferred::case_(2, [] { }));

  using result_type = decltype(expanded());
  static_assert(std::is_same_v<result_type, std::variant<int, std::monostate>>);

  CHECK(std::holds_alternative<std::monostate>(expanded()));
}
