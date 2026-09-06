// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <variant>

#include "deferred/detail/map_result.hpp"

TEST_CASE("map_result forwards a matching result", "[map-result]")
{
  auto const r = deferred::detail::map_result<int>([] { return 42; });

  STATIC_CHECK(std::is_same_v<decltype(r), int const>);
  CHECK(r == 42);
}

TEST_CASE("map_result maps a void result to monostate", "[map-result]")
{
  auto const r = deferred::detail::map_result<int>([] { });

  STATIC_CHECK(std::is_same_v<decltype(r), std::monostate const>);
}

TEST_CASE("map_result with a void target discards a non-void result", "[map-result]")
{
  STATIC_CHECK(std::is_void_v<decltype(deferred::detail::map_result<void>([] { return 42; }))>);
}

TEST_CASE("map_result with a void target and a void result", "[map-result]")
{
  STATIC_CHECK(std::is_void_v<decltype(deferred::detail::map_result<void>([] { }))>);
}

TEST_CASE("map_result invokes the function exactly once", "[map-result]")
{
  int calls = 0;

  SECTION("non-void target")
  {
    deferred::detail::map_result<int>([&] {
      ++calls;
      return 1;
    });
    CHECK(calls == 1);
  }

  SECTION("void target discarding a value")
  {
    deferred::detail::map_result<void>([&] {
      ++calls;
      return 1;
    });
    CHECK(calls == 1);
  }

  SECTION("void result mapped to monostate")
  {
    deferred::detail::map_result<int>([&] { ++calls; });
    CHECK(calls == 1);
  }
}

TEST_CASE("map_result preserves a reference result", "[map-result]")
{
  int value = 7;
  auto&& r  = deferred::detail::map_result<int&>([&]() -> int& { return value; });

  STATIC_CHECK(
    std::is_same_v<decltype(deferred::detail::map_result<int&>([&]() -> int& { return value; })),
                   int&>);
  CHECK(&r == &value);
}

TEST_CASE("map_result propagates the noexcept of its function", "[map-result]")
{
  auto throwing = [] {
    return 1;
  };
  auto nothrow = []() noexcept {
    return 1;
  };

  // A plain lambda is never implicitly noexcept, so the common call site is potentially-throwing.
  STATIC_CHECK(!noexcept(deferred::detail::map_result<int>(throwing)));
  STATIC_CHECK(noexcept(deferred::detail::map_result<int>(nothrow)));
}

TEST_CASE("map_result is usable in a constant expression", "[map-result]")
{
  constexpr auto r = deferred::detail::map_result<int>([] { return 5; });
  STATIC_CHECK(r == 5);
}
