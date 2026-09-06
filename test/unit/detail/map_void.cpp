// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <variant>

#include "deferred/detail/map_void.hpp"

TEST_CASE("map_void_t maps void to monostate", "[map-void]")
{
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<void>, std::monostate>);
}

TEST_CASE("map_void_t leaves non-void types alone", "[map-void]")
{
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<int>, int>);
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<int const>, int const>);
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<int&>, int&>);
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<int&&>, int&&>);
}

TEST_CASE("map_void_t maps cv-qualified void", "[map-void]")
{
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<void const>, std::monostate>);
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<void volatile>, std::monostate>);
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<void const volatile>, std::monostate>);
}

TEST_CASE("map_void_t does not treat void pointers as void", "[map-void]")
{
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<void*>, void*>);
  STATIC_CHECK(std::is_same_v<deferred::detail::map_void_t<void const*>, void const*>);
}

TEST_CASE("map_void_t is idempotent on monostate", "[map-void]")
{
  using once  = deferred::detail::map_void_t<void>;
  using twice = deferred::detail::map_void_t<once>;
  STATIC_CHECK(std::is_same_v<twice, std::monostate>);
}
