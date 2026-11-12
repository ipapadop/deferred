// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "deferred/deferred.hpp"

TEST_CASE("make_deferred_t leaves a deferred type alone", "[expression]")
{
  static_assert(
    std::is_same_v<deferred::make_deferred_t<deferred::constant<int>>, deferred::constant<int>>);

  // A deferred lvalue keeps its reference, so the expression refers to the original.
  static_assert(
    std::is_same_v<deferred::make_deferred_t<deferred::variable<int>&>, deferred::variable<int>&>);
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
  static_assert(std::is_same_v<deferred::make_deferred_t<int>, deferred::constant<int>>);
  static_assert(std::is_same_v<deferred::make_deferred_t<int&>, deferred::constant<int>>);
}

TEST_CASE("expression exposes its operator and subexpressions", "[expression]")
{
  auto ex = deferred::expression_<std::plus<>, deferred::constant<int>, deferred::constant<int>>(
    std::plus<>{},
    deferred::constant<int>(1),
    deferred::constant<int>(2));

  static_assert(std::is_same_v<decltype(ex)::operator_type, std::plus<>>);
  // Only the visited children are subexpressions; the operator is not one of them.
  static_assert(std::is_same_v<decltype(ex)::subexpression_types,
                               std::tuple<deferred::constant<int>, deferred::constant<int>>>);

  CHECK(std::get<0>(ex.subexpressions())() == 1);
  CHECK(ex() == 3);
}

TEST_CASE("expression evaluates its subexpressions on every evaluation", "[expression]")
{
  auto v  = deferred::variable(1);
  auto ex = deferred::expression_<std::plus<>, deferred::variable<int>&, deferred::constant<int>>(
    std::plus<>{},
    v,
    deferred::constant<int>(10));

  CHECK(ex() == 11);
  v = 5;
  CHECK(ex() == 15);
}

TEST_CASE("an expression with a stateful operator is not a constant expression", "[expression]")
{
  int state     = 0;
  auto stateful = [&state] {
    return state;
  };
  auto stateless = [] {
    return 1;
  };

  // The operator is no longer listed in subexpression_types, but it still counts.
  static_assert(deferred::is_constant_expression_v<deferred::expression_<decltype(stateless)>>);
  static_assert(!deferred::is_constant_expression_v<deferred::expression_<decltype(stateful)>>);
}

namespace {

template<template<typename...> class Node, typename... Arguments>
concept nameable = requires { typename Node<Arguments...>; };

} // namespace

TEST_CASE("an expression requires deferred subexpressions", "[expression]")
{
  // A template argument that is not a deferred expression is rejected at the class,
  // not when the expression is evaluated.
  static_assert(nameable<deferred::expression_, std::plus<>, deferred::constant<int>>);
  static_assert(!nameable<deferred::expression_, std::plus<>, int>);
  static_assert(!nameable<deferred::conditional_expression, deferred::detail::no_else, int>);
}

namespace {

/// @brief Whether subexpressions() of @p T returns references matching its
/// @c subexpression_types , element for element.
template<typename T>
consteval bool subexpressions_match()
{
  using children   = typename T::subexpression_types;
  using references = decltype(std::declval<T const&>().subexpressions());
  if (std::tuple_size_v<children> != std::tuple_size_v<references>)
  {
    return false;
  }
  return []<std::size_t... I>(std::index_sequence<I...>) {
    return (std::is_same_v<std::remove_cvref_t<std::tuple_element_t<I, children>>,
                           std::remove_cvref_t<std::tuple_element_t<I, references>>>
            && ...)
           && (std::is_lvalue_reference_v<std::tuple_element_t<I, references>> && ...);
  }(std::make_index_sequence<std::tuple_size_v<children>>{});
}

} // namespace

TEST_CASE("every node exposes its subexpressions", "[expression]")
{
  auto i = deferred::variable(1);
  auto c = deferred::constant(2);

  auto sum      = i + c;
  auto assigned = (i = c);
  auto both     = (i > 0 && c > 0);
  auto sequence = deferred::seq(i, c);
  auto loop     = deferred::while_(i < 0).do_(++i);
  auto do_loop  = deferred::do_(++i).while_(i < 0);
  auto for_loop = deferred::for_(i = deferred::constant(0), i < 0, ++i).do_(c);
  auto open_if  = deferred::if_(i > 0).then_(c);
  auto full_if  = deferred::if_(i > 0).then_(c).else_(i);
  auto open_sw  = deferred::switch_(i).case_(1).then_(c);
  auto full_sw  = deferred::switch_(i).case_(1).then_(c).default_(i);

  static_assert(subexpressions_match<decltype(i)>());
  static_assert(subexpressions_match<decltype(c)>());
  static_assert(subexpressions_match<decltype(sum)>());
  static_assert(subexpressions_match<decltype(assigned)>());
  static_assert(subexpressions_match<decltype(both)>());
  static_assert(subexpressions_match<decltype(sequence)>());
  static_assert(subexpressions_match<decltype(loop)>());
  static_assert(subexpressions_match<decltype(do_loop)>());
  static_assert(subexpressions_match<decltype(for_loop)>());
  static_assert(subexpressions_match<decltype(open_if)>());
  static_assert(subexpressions_match<decltype(full_if)>());
  static_assert(subexpressions_match<decltype(open_sw)>());
  static_assert(subexpressions_match<decltype(full_sw)>());

  // A leaf has none; a variable held by reference is handed back as that variable.
  static_assert(std::tuple_size_v<decltype(i.subexpressions())> == 0);
  CHECK(&std::get<0>(sum.subexpressions()) == &i);
  CHECK(&std::get<0>(assigned.subexpressions()) == &i);
  CHECK(std::get<1>(sum.subexpressions())() == 2);

  // The children of a branch and of a case are reachable the same way.
  auto const& branch = std::get<0>(full_if.subexpressions());
  CHECK(std::get<1>(branch.subexpressions())() == 2);
  auto const& first_case = std::get<2>(full_sw.subexpressions());
  CHECK(std::get<0>(first_case.subexpressions())() == 1);
}
