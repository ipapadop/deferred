// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <vector>

#include "deferred/constant.hpp"
#include "deferred/invoke.hpp"
#include "deferred/operators.hpp"

namespace {

template<typename T>
concept dereferenceable = requires(T t) { *t; };

template<typename T>
concept subscriptable = requires(T t) { deferred::subscript(t, 0); };

} // namespace

TEST_CASE("dereference and subscript require an operand that yields an lvalue", "[operators]")
{
  // Both hand back a reference into their operand, so an operand evaluating to a
  // prvalue would leave that reference dangling.
  using owned_pointer       = deferred::constant_<int*>&;
  using temporary_pointer   = decltype(deferred::invoke([] { return std::make_unique<int>(1); }))&;
  using owned_container     = deferred::constant_<std::vector<int>>&;
  using temporary_container = decltype(deferred::invoke([] { return std::vector<int>{1}; }))&;

  static_assert(dereferenceable<owned_pointer>);
  static_assert(!dereferenceable<temporary_pointer>);

  static_assert(subscriptable<owned_container>);
  static_assert(!subscriptable<temporary_container>);
}
