// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "deferred/conditional.hpp"
#include "deferred/detail/is_nothrow_evaluable.hpp"
#include "deferred/switch.hpp"

namespace {

struct probe_nothrow
{ };

struct probe_throwing
{ };

struct throwing_boolean
{
  explicit operator bool() const noexcept(false)
  {
    return true;
  }
};

} // namespace

namespace deferred::detail {

template<>
struct is_nothrow_evaluable<probe_nothrow> : std::true_type
{ };

template<>
struct is_nothrow_evaluable<probe_throwing> : std::false_type
{ };

} // namespace deferred::detail

TEST_CASE("is_nothrow_evaluable_v reads through to the specialization", "[is-nothrow-evaluable]")
{
  STATIC_CHECK(deferred::detail::is_nothrow_evaluable_v<probe_nothrow>);
  STATIC_CHECK(!deferred::detail::is_nothrow_evaluable_v<probe_throwing>);
}

TEST_CASE("is_nothrow_evaluable matches a conditional's own noexcept", "[is-nothrow-evaluable]")
{
  auto ex          = deferred::if_(true, 1).else_(2);
  using expression = decltype(ex);

  STATIC_CHECK(deferred::detail::is_nothrow_evaluable_v<expression const&> == noexcept(ex()));
  STATIC_CHECK(deferred::detail::is_nothrow_evaluable_v<expression const&>);
}

TEST_CASE("is_nothrow_evaluable tracks a throwing conditional condition", "[is-nothrow-evaluable]")
{
  auto ex          = deferred::if_(deferred::constant(throwing_boolean{}), 1).else_(2);
  using expression = decltype(ex);

  STATIC_CHECK(!deferred::detail::is_nothrow_evaluable_v<expression const&>);
  STATIC_CHECK(deferred::detail::is_nothrow_evaluable_v<expression const&> == noexcept(ex()));
}

TEST_CASE("is_nothrow_evaluable distinguishes const and mutable references",
          "[is-nothrow-evaluable]")
{
  auto ex          = deferred::if_(true, 1).else_(2);
  using expression = decltype(ex);

  STATIC_CHECK(deferred::detail::is_nothrow_evaluable_v<expression&>);
  STATIC_CHECK(deferred::detail::is_nothrow_evaluable_v<expression const&>);
}

TEST_CASE("is_nothrow_evaluable matches a switch's own noexcept", "[is-nothrow-evaluable]")
{
  auto ex          = deferred::switch_(1, deferred::default_(0), deferred::case_(1, 10));
  using expression = decltype(ex);

  STATIC_CHECK(deferred::detail::is_nothrow_evaluable_v<expression const&> == noexcept(ex()));
}
