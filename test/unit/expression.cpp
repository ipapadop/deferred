// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <tuple>
#include <type_traits>

#include "deferred/constant.hpp"
#include "deferred/expression.hpp"
#include "deferred/variable.hpp"

TEST_CASE("make_deferred_t leaves a deferred type alone", "[expression]")
{
  static_assert(
    std::is_same_v<deferred::make_deferred_t<deferred::constant_<int>>, deferred::constant_<int>>);

  // A deferred lvalue keeps its reference, so the expression refers to the original.
  static_assert(
    std::is_same_v<deferred::make_deferred_t<deferred::variable_<int>&>, deferred::variable_<int>&>);
}

TEST_CASE("make_deferred_t wraps a callable in an expression", "[expression]")
{
  auto callable = [] {
    return 1;
  };

  static_assert(std::is_same_v<deferred::make_deferred_t<decltype(callable)>,
                               deferred::expression_<decltype(callable)>>);
}

TEST_CASE("make_deferred_t wraps a value in a constant", "[expression]")
{
  static_assert(std::is_same_v<deferred::make_deferred_t<int>, deferred::constant_<int>>);
  static_assert(std::is_same_v<deferred::make_deferred_t<int&>, deferred::constant_<int>>);
}

TEST_CASE("expression exposes its operator and subexpressions", "[expression]")
{
  auto ex = deferred::expression_<std::plus<>, deferred::constant_<int>, deferred::constant_<int>>(
    std::plus<>{},
    deferred::constant_<int>(1),
    deferred::constant_<int>(2));

  static_assert(std::is_same_v<decltype(ex)::operator_type, std::plus<>>);
  static_assert(std::is_same_v<decltype(ex)::expression_types,
                               std::tuple<deferred::constant_<int>, deferred::constant_<int>>>);

  CHECK(std::get<0>(ex.subexpressions())() == 1);
  CHECK(ex() == 3);
}

TEST_CASE("expression evaluates its subexpressions on every evaluation", "[expression]")
{
  auto v  = deferred::variable(1);
  auto ex = deferred::expression_<std::plus<>, deferred::variable_<int>&, deferred::constant_<int>>(
    std::plus<>{},
    v,
    deferred::constant_<int>(10));

  CHECK(ex() == 11);
  v = 5;
  CHECK(ex() == 15);
}
