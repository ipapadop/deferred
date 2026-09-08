// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "deferred/constant.hpp"
#include "deferred/evaluate.hpp"
#include "deferred/variable.hpp"

namespace {

struct void_node
{
  using subexpression_types = std::tuple<>;

  constexpr void operator()() const noexcept
  { }
};

struct throwing_node
{
  using subexpression_types = std::tuple<>;

  constexpr int operator()() const noexcept(false)
  {
    return 1;
  }
};

struct narrowing_target
{
  narrowing_target(int) noexcept(false);
};

struct throwing_move
{
  throwing_move()                     = default;
  throwing_move(throwing_move const&) = default;
  throwing_move(throwing_move&&) noexcept(false);
};

struct nothrow_copy
{
  nothrow_copy()                    = default;
  nothrow_copy(nothrow_copy const&) = default;
};

template<typename Result, typename Ref>
inline constexpr bool result_nothrow_v =
  deferred::detail::evaluation_result_is_nothrow<Result, Ref>();

template<template<typename> class Node, typename T>
inline constexpr bool make_nothrow_v = deferred::detail::make_node_is_nothrow<Node, T>();

} // namespace

TEST_CASE("evaluation_result_is_nothrow for a plain constant", "[evaluate-noexcept]")
{
  STATIC_CHECK(result_nothrow_v<int, deferred::constant_<int>&>);
  STATIC_CHECK(result_nothrow_v<int, deferred::constant_<int> const&>);
}

TEST_CASE("evaluation_result_is_nothrow detects a throwing evaluation", "[evaluate-noexcept]")
{
  STATIC_CHECK(!result_nothrow_v<int, throwing_node&>);
}

TEST_CASE("evaluation_result_is_nothrow detects a throwing result construction",
          "[evaluate-noexcept]")
{
  STATIC_CHECK(!result_nothrow_v<narrowing_target, deferred::constant_<int>&>);
}

TEST_CASE("evaluation_result_is_nothrow maps a void evaluation to monostate", "[evaluate-noexcept]")
{
  STATIC_CHECK(result_nothrow_v<std::variant<int, std::monostate>, void_node&>);
  STATIC_CHECK(result_nothrow_v<std::monostate, void_node&>);
}

TEST_CASE("evaluation_result_is_nothrow is false for a void result type", "[evaluate-noexcept]")
{
  // void is not constructible, so callers must special-case a void result rather than
  // asking this trait about it.
  STATIC_CHECK(!result_nothrow_v<void, deferred::constant_<int>&>);
  STATIC_CHECK(!result_nothrow_v<void, void_node&>);
}

TEST_CASE("evaluation_result_is_nothrow widens to a variant result", "[evaluate-noexcept]")
{
  STATIC_CHECK(result_nothrow_v<std::variant<int, double>, deferred::constant_<int>&>);
}

TEST_CASE("make_node_is_nothrow for a trivially stored value", "[evaluate-noexcept]")
{
  STATIC_CHECK(make_nothrow_v<deferred::constant_, int>);
  STATIC_CHECK(make_nothrow_v<deferred::variable_, int>);
  STATIC_CHECK(make_nothrow_v<deferred::constant_, nothrow_copy>);
}

TEST_CASE("make_node_is_nothrow detects a throwing move", "[evaluate-noexcept]")
{
  STATIC_CHECK(!make_nothrow_v<deferred::constant_, throwing_move>);
  STATIC_CHECK(!make_nothrow_v<deferred::variable_, throwing_move>);
}

TEST_CASE("make_node_is_nothrow decays arrays rather than rejecting them", "[evaluate-noexcept]")
{
  // recursive_evaluate returns by value, so an array argument decays to a pointer;
  // the trait must model the decayed type, not the array itself.
  STATIC_CHECK(make_nothrow_v<deferred::constant_, char const(&)[6]>);
  STATIC_CHECK(make_nothrow_v<deferred::variable_, char const(&)[6]>);
  STATIC_CHECK(make_nothrow_v<deferred::constant_, int (&)[4]>);
  STATIC_CHECK(noexcept(deferred::evaluate("hello")));
  STATIC_CHECK(noexcept(deferred::constant("hello")));
  STATIC_CHECK(noexcept(deferred::variable("hello")));
}

TEST_CASE("make_node_is_nothrow follows a throwing callable", "[evaluate-noexcept]")
{
  auto nothrow_fn = []() noexcept {
    return 1;
  };
  auto throwing_fn = [] {
    return 1;
  };

  STATIC_CHECK(make_nothrow_v<deferred::constant_, decltype(nothrow_fn)>);
  STATIC_CHECK(!make_nothrow_v<deferred::constant_, decltype(throwing_fn)>);
}

TEST_CASE("make_node_is_nothrow follows nested callables", "[evaluate-noexcept]")
{
  // recursive_evaluate keeps invoking, so a throw at any depth is observed.
  auto nested_nothrow = []() noexcept {
    return []() noexcept {
      return 1;
    };
  };
  auto nested_throwing = []() noexcept {
    return [] {
      return 1;
    };
  };

  STATIC_CHECK(make_nothrow_v<deferred::constant_, decltype(nested_nothrow)>);
  STATIC_CHECK(!make_nothrow_v<deferred::constant_, decltype(nested_throwing)>);
}

TEST_CASE("make_node_is_nothrow agrees with the factory it guards", "[evaluate-noexcept]")
{
  STATIC_CHECK(noexcept(deferred::constant(1)) == make_nothrow_v<deferred::constant_, int>);
  STATIC_CHECK(noexcept(deferred::constant(std::declval<throwing_move>()))
               == make_nothrow_v<deferred::constant_, throwing_move>);
}

TEST_CASE("evaluated_result_t is the type evaluate() yields", "[evaluate-result]")
{
  STATIC_CHECK(std::is_same_v<deferred::detail::evaluated_result_t<deferred::constant_<int>>, int>);
  STATIC_CHECK(std::is_same_v<deferred::detail::evaluated_result_t<void_node>, void>);
}

TEST_CASE("evaluated_result_t strips the references operator() returns", "[evaluate-result]")
{
  // variable_ returns int& for an lvalue and constant_ returns int const&, but
  // evaluate() yields a value in both cases.
  STATIC_CHECK(
    std::is_same_v<deferred::detail::evaluated_result_t<deferred::variable_<int>&>, int>);
  STATIC_CHECK(
    std::is_same_v<deferred::detail::evaluated_result_t<deferred::constant_<int>&>, int>);
}

TEST_CASE("evaluated_result_t follows a nested deferred expression", "[evaluate-result]")
{
  using nested = deferred::constant_<deferred::constant_<int>>;

  STATIC_CHECK(std::is_same_v<deferred::detail::evaluated_result_t<nested>, int>);
}
