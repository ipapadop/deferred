// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

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
  auto i = 0;
  auto j = 0;
  auto k = 0;
  auto ex =
    deferred::if_then_else([&] { return i++ == 1; }, [&] { return ++j; }, [&] { return k += 2; });

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

TEST_CASE("if_then with literal", "[if_then-literal]")
{
  auto ex_true  = deferred::if_then(true, 42);
  auto res_true = ex_true();
  static_assert(std::is_same_v<decltype(res_true), std::optional<int>>);
  CHECK(res_true.has_value());
  CHECK(*res_true == 42);

  auto ex_false  = deferred::if_then(false, 42);
  auto res_false = ex_false();
  CHECK(!res_false.has_value());
}

TEST_CASE("if_then with void return", "[if_then-void]")
{
  int i   = 0;
  auto ex = deferred::if_then(true, [&] { ++i; });
  static_assert(std::is_void_v<decltype(ex())>);
  ex();
  CHECK(i == 1);

  auto ex2 = deferred::if_then(false, [&] { ++i; });
  ex2();
  CHECK(i == 1);
}

TEST_CASE("if_then with constexpr", "[if_then-constexpr]")
{
  constexpr auto ex = deferred::if_then(true, 42);
  static_assert(ex().has_value());
  static_assert(*ex() == 42);

  constexpr auto ex_false = deferred::if_then(false, 42);
  static_assert(!ex_false().has_value());
}

TEST_CASE("if_then visit", "[if_then-visit]")
{
  auto ex   = deferred::if_then(true, 42);
  int count = 0;
  ex.visit([&](auto const&, std::size_t) { ++count; });
  // 1 (if_then_expression) + 1 (condition) + 1 (then) = 3
  CHECK(count == 3);
}
