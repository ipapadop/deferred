// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/constant.hpp"
#include "deferred/type_traits/is_deferred.hpp"
#include "deferred/variable.hpp"

#include <type_traits>
#include <vector>

namespace {

struct has_types
{
  using subexpression_types = int;
};

} // namespace

TEST_CASE("is_deferred trait and deferred_expression concept", "[is_deferred]")
{
  using deferred::deferred_expression;
  using deferred::is_deferred;
  using deferred::is_deferred_t;
  using deferred::is_deferred_v;

  SECTION("deferred types")
  {
    using C = decltype(deferred::constant(42));
    static_assert(deferred_expression<C>);
    static_assert(is_deferred<C>::value);
    static_assert(is_deferred_v<C>);
    static_assert(std::is_same_v<is_deferred_t<C>, std::true_type>);

    using V = decltype(deferred::variable(42));
    static_assert(deferred_expression<V>);
    static_assert(is_deferred_v<V>);
  }

  SECTION("non-deferred types")
  {
    static_assert(!deferred_expression<int>);
    static_assert(!is_deferred_v<int>);

    static_assert(!deferred_expression<std::vector<int>>);
    static_assert(!is_deferred_v<std::vector<int>>);

    struct dummy
    { };
    static_assert(!deferred_expression<dummy>);
    static_assert(!is_deferred_v<dummy>);
  }

  SECTION("cv-qualified and reference types")
  {
    using C = decltype(deferred::constant(42));

    static_assert(deferred_expression<C const>);
    static_assert(is_deferred_v<C const>);

    static_assert(deferred_expression<C&>);
    static_assert(is_deferred_v<C&>);

    static_assert(deferred_expression<C const&>);
    static_assert(is_deferred_v<C const&>);

    static_assert(deferred_expression<C&&>);
    static_assert(is_deferred_v<C&&>);

    static_assert(deferred_expression<C volatile>);
    static_assert(is_deferred_v<C volatile>);

    static_assert(deferred_expression<C const volatile&>);
    static_assert(is_deferred_v<C const volatile&>);
  }
}

TEST_CASE("any_deferred concept", "[is_deferred]")
{
  using deferred::any_deferred;

  using C = decltype(deferred::constant(42));
  using V = decltype(deferred::variable(42));

  static_assert(any_deferred<C>);
  static_assert(any_deferred<V>);
  static_assert(any_deferred<C, int>);
  static_assert(any_deferred<int, V>);
  static_assert(any_deferred<C, V>);
  static_assert(any_deferred<int, C, float>);

  static_assert(!any_deferred<>);
  static_assert(!any_deferred<int>);
  static_assert(!any_deferred<int, float, double>);
  static_assert(!any_deferred<std::vector<int>>);
}

TEST_CASE("has_subexpression_types concept", "[is_deferred]")
{
  using deferred::has_subexpression_types;

  static_assert(has_subexpression_types<has_types>);
  static_assert(has_subexpression_types<decltype(deferred::constant(42))>);
  static_assert(!has_subexpression_types<int>);

  // Unlike deferred_expression, it does not look through cv-qualifiers and references.
  static_assert(!has_subexpression_types<has_types&>);
  static_assert(deferred::deferred_expression<has_types&>);
}
