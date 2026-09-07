// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "deferred/detail/callable_storage.hpp"

namespace {

constexpr int add(int a, int b)
{
  return a + b;
}

struct functor
{
  constexpr int operator()() const
  {
    return 42;
  }
};

struct widget
{
  int value;

  constexpr int get() const
  {
    return value;
  }
};

} // namespace

TEST_CASE("callable_storage_t holds function objects by reference", "[callable-storage]")
{
  STATIC_CHECK(std::is_same_v<deferred::detail::callable_storage_t<functor&>, functor&>);
  STATIC_CHECK(
    std::is_same_v<deferred::detail::callable_storage_t<functor const&>, functor const&>);
  STATIC_CHECK(std::is_same_v<deferred::detail::callable_storage_t<functor>, functor&&>);
}

TEST_CASE("callable_storage_t holds functions as pointers", "[callable-storage]")
{
  using function_type = decltype(add);
  STATIC_CHECK(
    std::is_same_v<deferred::detail::callable_storage_t<function_type&>, function_type*>);
  STATIC_CHECK(std::is_same_v<deferred::detail::callable_storage_t<function_type>, function_type*>);
}

TEST_CASE("callable_storage_t holds member pointers by reference", "[callable-storage]")
{
  using member = decltype(&widget::get);
  STATIC_CHECK(std::is_same_v<deferred::detail::callable_storage_t<member&>, member&>);
  STATIC_CHECK(std::is_same_v<deferred::detail::callable_storage_t<member>, member&&>);
}

TEST_CASE("a stored function is usable in a constant expression", "[callable-storage]")
{
  struct holder
  {
    deferred::detail::callable_storage_t<decltype(add)&> f;
  };

  constexpr holder h{add};
  STATIC_CHECK(h.f(1, 2) == 3);
}
