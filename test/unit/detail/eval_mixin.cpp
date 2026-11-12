// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <utility>

#include "deferred/detail/eval_mixin.hpp"

namespace {

struct node : deferred::detail::eval_mixin<node>
{
  int value = 0;

  constexpr int const& operator()() const noexcept
  {
    return value;
  }

  constexpr int& operator()() noexcept(false)
  {
    return value;
  }
};

struct const_only_node : deferred::detail::eval_mixin<const_only_node>
{
  constexpr int operator()() const noexcept
  {
    return 42;
  }
};

struct effect_node : deferred::detail::eval_mixin<effect_node>
{
  int* calls;

  constexpr void operator()() const
  {
    ++*calls;
  }
};

} // namespace

TEST_CASE("eval forwards to the call operator of the same constness", "[detail-eval-mixin]")
{
  node n;

  static_assert(std::is_same_v<decltype(n.eval()), int&>);
  static_assert(std::is_same_v<decltype(std::as_const(n).eval()), int const&>);
  static_assert(std::is_same_v<decltype(std::move(n).eval()), int&>);

  n.eval() = 7;
  CHECK(std::as_const(n).eval() == 7);
  CHECK(&n.eval() == &n.value);
}

TEST_CASE("eval has the exception specification of the call operator", "[detail-eval-mixin]")
{
  node n;

  static_assert(!noexcept(n.eval()));
  static_assert(noexcept(std::as_const(n).eval()));
}

TEST_CASE("eval on a mutable node uses a const call operator when it is the only one",
          "[detail-eval-mixin]")
{
  const_only_node n;

  static_assert(std::is_same_v<decltype(n.eval()), int>);
  static_assert(noexcept(n.eval()));
  static_assert(const_only_node{}.eval() == 42);
  CHECK(n.eval() == 42);
}

TEST_CASE("eval can be called for its effect", "[detail-eval-mixin]")
{
  auto calls = 0;
  effect_node n{{}, &calls};

  static_assert(std::is_void_v<decltype(n.eval())>);
  n.eval();
  CHECK(calls == 1);
}

TEST_CASE("the mixin adds no storage", "[detail-eval-mixin]")
{
  static_assert(sizeof(node) == sizeof(int));
  static_assert(std::is_empty_v<const_only_node>);
}
