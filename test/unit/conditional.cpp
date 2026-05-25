// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <variant>

#include "deferred/conditional.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"

TEST_CASE("conditional with literal", "[conditional-literal]")
{
  auto ex = deferred::if_(true, 42).else_(10);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 42);
}

TEST_CASE("conditional with constexpr", "[conditional-constexpr]")
{
  constexpr auto ex = deferred::if_(true, 42).else_(10);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(ex() == 42, "constexpr failed");
  CHECK(ex() == 42);
}

TEST_CASE("conditional with different result types", "[conditional-diff-types]")
{
  auto ex = deferred::if_(true, 42).else_(false);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(std::get<int>(ex()) == 42);
}

TEST_CASE("conditional with lambda", "[conditional-lambdas]")
{
  auto i = 0;
  auto j = 0;
  auto k = 0;
  auto ex =
    deferred::if_([&] { return i++ == 1; }, [&] { return ++j; }).else_([&] { return k += 2; });

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
  auto ex = deferred::if_([i]() mutable { return i++ == 1; }, [j]() mutable { return ++j; })
              .else_([k]() mutable { return k += 2; });

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
  CHECK(ex() == 2);
  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
}

TEST_CASE("if_ with literal", "[if-literal]")
{
  auto ex_true  = deferred::if_(true, 42);
  auto res_true = ex_true();
  static_assert(std::is_same_v<decltype(res_true), std::optional<int>>);
  CHECK(res_true.has_value());
  CHECK(*res_true == 42);

  auto ex_false  = deferred::if_(false, 42);
  auto res_false = ex_false();
  CHECK(!res_false.has_value());
}

TEST_CASE("if_ with void return", "[if-void]")
{
  int i   = 0;
  auto ex = deferred::if_(true, [&] { ++i; });
  static_assert(std::is_void_v<decltype(ex())>);
  ex();
  CHECK(i == 1);

  auto ex2 = deferred::if_(false, [&] { ++i; });
  ex2();
  CHECK(i == 1);
}

TEST_CASE("if_ with constexpr", "[if-constexpr]")
{
  constexpr auto ex = deferred::if_(true, 42);
  static_assert(ex().has_value());
  static_assert(*ex() == 42);

  constexpr auto ex_false = deferred::if_(false, 42);
  static_assert(!ex_false().has_value());
}

TEST_CASE("if_ visit", "[if-visit]")
{
  auto ex   = deferred::if_(true, 42);
  int count = 0;
  ex.visit([&](auto const&, std::size_t) { ++count; });
  // 1 (if_expression) + 1 (condition) + 1 (then) = 3
  CHECK(count == 3);
}

TEST_CASE("conditional with else_if", "[conditional-else-if]")
{
  auto ex = deferred::if_(false, 1).else_if(true, 2).else_(3);
  CHECK(ex() == 2);

  auto ex2 = deferred::if_(false, 1).else_if(false, 2).else_(3);
  CHECK(ex2() == 3);
}

TEST_CASE("conditional with mixed types (variant)", "[conditional-variant]")
{
  auto ex = deferred::if_(true, 42).else_("hello");

  using result_type = decltype(ex());
  static_assert(std::is_same_v<result_type, std::variant<int, char const*>>);

  auto res = ex();
  CHECK(std::get<int>(res) == 42);

  auto ex2  = deferred::if_(false, 42).else_("hello");
  auto res2 = ex2();
  CHECK(std::get<char const*>(res2) == std::string("hello"));
}

TEST_CASE("if_ with multiple else_if (optional)", "[if-else-if-optional]")
{
  auto ex  = deferred::if_(false, 1).else_if(true, 2).else_if(true, 3);
  auto res = ex();
  CHECK(res.has_value());
  CHECK(*res == 2);

  auto ex2  = deferred::if_(false, 1).else_if(false, 2);
  auto res2 = ex2();
  CHECK(!res2.has_value());
}
