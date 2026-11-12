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

// Whether constructing a Node, with its type deduced, from a T cannot throw.
template<template<typename> class Node, typename T>
inline constexpr bool make_nothrow_v = noexcept(Node(std::declval<T>()));

} // namespace

TEST_CASE("evaluation_result_is_nothrow for a plain constant", "[evaluate-noexcept]")
{
  STATIC_CHECK(result_nothrow_v<int, deferred::constant<int>&>);
  STATIC_CHECK(result_nothrow_v<int, deferred::constant<int> const&>);
}

TEST_CASE("evaluation_result_is_nothrow detects a throwing evaluation", "[evaluate-noexcept]")
{
  STATIC_CHECK(!result_nothrow_v<int, throwing_node&>);
}

TEST_CASE("evaluation_result_is_nothrow detects a throwing result construction",
          "[evaluate-noexcept]")
{
  STATIC_CHECK(!result_nothrow_v<narrowing_target, deferred::constant<int>&>);
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
  STATIC_CHECK(!result_nothrow_v<void, deferred::constant<int>&>);
  STATIC_CHECK(!result_nothrow_v<void, void_node&>);
}

TEST_CASE("evaluation_result_is_nothrow widens to a variant result", "[evaluate-noexcept]")
{
  STATIC_CHECK(result_nothrow_v<std::variant<int, double>, deferred::constant<int>&>);
}

TEST_CASE("constructing a node is noexcept for a trivially stored value", "[evaluate-noexcept]")
{
  STATIC_CHECK(make_nothrow_v<deferred::constant, int>);
  STATIC_CHECK(make_nothrow_v<deferred::variable, int>);
  STATIC_CHECK(make_nothrow_v<deferred::constant, nothrow_copy>);
}

TEST_CASE("constructing a node detects a throwing move", "[evaluate-noexcept]")
{
  STATIC_CHECK(!make_nothrow_v<deferred::constant, throwing_move>);
  STATIC_CHECK(!make_nothrow_v<deferred::variable, throwing_move>);
}

TEST_CASE("constructing a node decays arrays rather than rejecting them", "[evaluate-noexcept]")
{
  // recursive_evaluate returns by value, so an array argument decays to a pointer;
  // the trait must model the decayed type, not the array itself.
  STATIC_CHECK(make_nothrow_v<deferred::constant, char const(&)[6]>);
  STATIC_CHECK(make_nothrow_v<deferred::variable, char const(&)[6]>);
  STATIC_CHECK(make_nothrow_v<deferred::constant, int (&)[4]>);
  STATIC_CHECK(noexcept(deferred::evaluate("hello")));
  STATIC_CHECK(noexcept(deferred::constant("hello")));
  STATIC_CHECK(noexcept(deferred::variable("hello")));
}

TEST_CASE("constructing a node follows a throwing callable", "[evaluate-noexcept]")
{
  auto nothrow_fn = []() noexcept {
    return 1;
  };
  auto throwing_fn = [] {
    return 1;
  };

  STATIC_CHECK(make_nothrow_v<deferred::constant, decltype(nothrow_fn)>);
  STATIC_CHECK(!make_nothrow_v<deferred::constant, decltype(throwing_fn)>);
}

TEST_CASE("constructing a node follows nested callables", "[evaluate-noexcept]")
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

  STATIC_CHECK(make_nothrow_v<deferred::constant, decltype(nested_nothrow)>);
  STATIC_CHECK(!make_nothrow_v<deferred::constant, decltype(nested_throwing)>);
}

TEST_CASE("deduced construction agrees with the spelled-out check", "[evaluate-noexcept]")
{
  STATIC_CHECK(noexcept(deferred::constant(1)) == make_nothrow_v<deferred::constant, int>);
  STATIC_CHECK(noexcept(deferred::constant(std::declval<throwing_move>()))
               == make_nothrow_v<deferred::constant, throwing_move>);
}

TEST_CASE("evaluated_result_t is the type evaluate() yields", "[evaluate-result]")
{
  STATIC_CHECK(std::is_same_v<deferred::detail::evaluated_result_t<deferred::constant<int>>, int>);
  STATIC_CHECK(std::is_same_v<deferred::detail::evaluated_result_t<void_node>, void>);
}

TEST_CASE("evaluated_result_t strips the references operator() returns", "[evaluate-result]")
{
  // variable returns int& for an lvalue and constant returns int const&, but
  // evaluate() yields a value in both cases.
  STATIC_CHECK(std::is_same_v<deferred::detail::evaluated_result_t<deferred::variable<int>&>, int>);
  STATIC_CHECK(std::is_same_v<deferred::detail::evaluated_result_t<deferred::constant<int>&>, int>);
}

TEST_CASE("evaluated_result_t follows a nested deferred expression", "[evaluate-result]")
{
  using nested = deferred::constant<deferred::constant<int>>;

  STATIC_CHECK(std::is_same_v<deferred::detail::evaluated_result_t<nested>, int>);
}

TEST_CASE("evaluate_ref keeps a reference result as a reference", "[evaluate-ref]")
{
  auto v       = deferred::variable(1);
  auto const c = deferred::constant(2);

  static_assert(std::is_same_v<decltype(deferred::evaluate_ref(v)), int&>);
  static_assert(std::is_same_v<decltype(deferred::evaluate_ref(c)), int const&>);
  static_assert(std::is_same_v<decltype(deferred::evaluate(v)), int>);

  CHECK(&deferred::evaluate_ref(v) == &v());
}

TEST_CASE("evaluate_ref keeps a prvalue result as a value", "[evaluate-ref]")
{
  static_assert(std::is_same_v<decltype(deferred::evaluate_ref(throwing_node{})), int>);
  static_assert(std::is_void_v<decltype(deferred::evaluate_ref(void_node{}))>);
}

TEST_CASE("evaluate_ref does not return a reference to a non-deferred temporary", "[evaluate-ref]")
{
  int i = 1;

  static_assert(std::is_same_v<decltype(deferred::evaluate_ref(i)), int&>);
  static_assert(std::is_same_v<decltype(deferred::evaluate_ref(std::as_const(i))), int const&>);
  static_assert(std::is_same_v<decltype(deferred::evaluate_ref(42)), int>);

  CHECK(deferred::evaluate_ref(42) == 42);
}

TEST_CASE("evaluate_ref evaluates a nested deferred expression by value", "[evaluate-ref]")
{
  struct nested_node
  {
    using subexpression_types = std::tuple<>;

    constexpr deferred::constant<int> operator()() const noexcept
    {
      return deferred::constant<int>(7);
    }
  };

  static_assert(std::is_same_v<decltype(deferred::evaluate_ref(nested_node{})), int>);
  CHECK(deferred::evaluate_ref(nested_node{}) == 7);
}

TEST_CASE("evaluate_ref is noexcept unless the evaluation can throw", "[evaluate-ref]")
{
  auto v = deferred::variable<throwing_move>();

  // Nothing is copied or moved, so a throwing move does not matter.
  static_assert(noexcept(deferred::evaluate_ref(v)));
  static_assert(noexcept(deferred::evaluate_ref(void_node{})));
  static_assert(!noexcept(deferred::evaluate_ref(throwing_node{})));
  // A non-deferred rvalue is moved into the result.
  static_assert(!noexcept(deferred::evaluate_ref(throwing_move{})));
}
