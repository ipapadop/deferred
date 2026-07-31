// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/conditional.hpp"
#include "deferred/constant.hpp"
#include "deferred/type_traits/is_deferred.hpp"
#include "deferred/variable.hpp"

#include <type_traits>
#include <vector>

TEST_CASE("is_deferred trait and Deferred concept", "[is_deferred]")
{
  using deferred::Deferred;
  using deferred::is_deferred;
  using deferred::is_deferred_t;
  using deferred::is_deferred_v;

  SECTION("deferred types")
  {
    using C = decltype(deferred::constant(42));
    static_assert(Deferred<C>);
    static_assert(is_deferred<C>::value);
    static_assert(is_deferred_v<C>);
    static_assert(std::is_same_v<is_deferred_t<C>, std::true_type>);

    using V = decltype(deferred::variable(42));
    static_assert(Deferred<V>);
    static_assert(is_deferred_v<V>);

    using I = decltype(deferred::if_then_else(true, 1, 2));
    static_assert(Deferred<I>);
    static_assert(is_deferred_v<I>);
  }

  SECTION("non-deferred types")
  {
    static_assert(!Deferred<int>);
    static_assert(!is_deferred_v<int>);

    static_assert(!Deferred<std::vector<int>>);
    static_assert(!is_deferred_v<std::vector<int>>);

    struct dummy
    { };
    static_assert(!Deferred<dummy>);
    static_assert(!is_deferred_v<dummy>);
  }

  SECTION("cv-qualified and reference types")
  {
    using C = decltype(deferred::constant(42));

    static_assert(Deferred<C const>);
    static_assert(is_deferred_v<C const>);

    static_assert(Deferred<C&>);
    static_assert(is_deferred_v<C&>);

    static_assert(Deferred<C const&>);
    static_assert(is_deferred_v<C const&>);

    static_assert(Deferred<C&&>);
    static_assert(is_deferred_v<C&&>);

    static_assert(Deferred<C volatile>);
    static_assert(is_deferred_v<C volatile>);

    static_assert(Deferred<C const volatile&>);
    static_assert(is_deferred_v<C const volatile&>);
  }
}

TEST_CASE("AnyDeferred concept", "[is_deferred]")
{
  using deferred::AnyDeferred;

  using C = decltype(deferred::constant(42));
  using V = decltype(deferred::variable(42));

  static_assert(AnyDeferred<C>);
  static_assert(AnyDeferred<V>);
  static_assert(AnyDeferred<C, int>);
  static_assert(AnyDeferred<int, V>);
  static_assert(AnyDeferred<C, V>);
  static_assert(AnyDeferred<int, C, float>);

  static_assert(!AnyDeferred<>);
  static_assert(!AnyDeferred<int>);
  static_assert(!AnyDeferred<int, float, double>);
  static_assert(!AnyDeferred<std::vector<int>>);
}
