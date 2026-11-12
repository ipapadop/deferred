// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <type_traits>
#include <vector>

#include "deferred/constant.hpp"
#include "deferred/invoke.hpp"
#include "deferred/operators.hpp"
#include "deferred/variable.hpp"

namespace {

template<typename T>
concept dereferenceable = requires(T t) { *t; };

template<typename T>
concept subscriptable = requires(T t) { deferred::subscript(t, 0); };

} // namespace

TEST_CASE("dereference and subscript accept an operand that yields a prvalue", "[operators]")
{
  using owned_pointer       = deferred::constant_<int*>&;
  using temporary_pointer   = decltype(deferred::invoke([] { return std::make_unique<int>(1); }))&;
  using owned_container     = deferred::constant_<std::vector<int>>&;
  using temporary_container = decltype(deferred::invoke([] { return std::vector<int>{1}; }))&;

  static_assert(dereferenceable<owned_pointer>);
  static_assert(dereferenceable<temporary_pointer>);

  static_assert(subscriptable<owned_container>);
  static_assert(subscriptable<temporary_container>);
}

TEST_CASE("dereference and subscript yield a reference only when it cannot dangle", "[operators]")
{
  int values[]   = {1, 2, 3};
  auto p         = deferred::variable<int*>(values);
  auto container = deferred::variable(std::vector<int>{4, 5});

  // An lvalue operand: the result refers into it.
  static_assert(std::is_same_v<decltype((*p)()), int&>);
  static_assert(std::is_same_v<decltype(deferred::subscript(container, 0)()), int&>);
  CHECK(&(*p)() == &values[0]);

  // A raw pointer does not own what it points to, so a pointer computed during the
  // evaluation still yields a reference.
  static_assert(std::is_same_v<decltype((*(p + 1))()), int&>);
  static_assert(std::is_same_v<decltype(deferred::subscript(p + 1, 1)()), int&>);
  CHECK(&(*(p + 1))() == &values[1]);
  CHECK(&deferred::subscript(p + 1, 1)() == &values[2]);

  // Any other temporary owns its result, which is therefore returned by value.
  auto owner  = deferred::invoke([] { return std::make_unique<int>(7); });
  auto vector = deferred::invoke([] { return std::vector<int>{8, 9}; });
  static_assert(std::is_same_v<decltype((*owner)()), int>);
  static_assert(std::is_same_v<decltype(deferred::subscript(vector, 1)()), int>);
  CHECK((*owner)() == 7);
  CHECK(deferred::subscript(vector, 1)() == 9);
}

TEST_CASE("prefix increment and decrement yield an lvalue when the operand does", "[operators]")
{
  auto i = deferred::variable(1);

  static_assert(std::is_same_v<decltype((++i)()), int&>);
  static_assert(std::is_same_v<decltype((--i)()), int&>);
  // The postfix forms yield the previous value, as the built-in ones do.
  static_assert(std::is_same_v<decltype((i++)()), int>);

  CHECK(&(++i)() == &i());
  CHECK(i() == 2);
}
