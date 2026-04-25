// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <tuple>

#include "deferred/apply.hpp"
#include "deferred/constant.hpp"
#include "deferred/variable.hpp"

namespace {

constexpr int add(int a, int b)
{
  return a + b;
}

struct multiply_t
{
  constexpr int operator()(int a, int b) const
  {
    return a * b;
  }
};

} // namespace

TEST_CASE("apply with constants", "[apply]")
{
  SECTION("basic usage")
  {
    auto t   = std::make_tuple(deferred::constant(1), deferred::constant(2));
    auto res = deferred::apply(add, t);
    CHECK(res == 3);
  }

  SECTION("function object")
  {
    auto t   = std::make_tuple(deferred::constant(3), deferred::constant(4));
    auto res = deferred::apply(multiply_t{}, t);
    CHECK(res == 12);
  }

  SECTION("lambda")
  {
    auto t   = std::make_tuple(deferred::constant(10), deferred::constant(5));
    auto res = deferred::apply([](auto a, auto b) { return a - b; }, t);
    CHECK(res == 5);
  }
}

TEST_CASE("apply with variables", "[apply]")
{
  auto v1 = deferred::variable<int>(10);
  auto v2 = deferred::variable<int>(20);
  auto t  = std::forward_as_tuple(v1, v2);

  SECTION("initial values")
  {
    CHECK(deferred::apply(add, t) == 30);
  }

  SECTION("updated values")
  {
    v1 = 30;
    v2 = 40;
    CHECK(deferred::apply(add, t) == 70);
  }
}

TEST_CASE("apply with mixed types", "[apply]")
{
  auto v = deferred::variable<int>(10);
  auto c = deferred::constant(5);
  auto t = std::forward_as_tuple(v, c);

  CHECK(deferred::apply(add, t) == 15);

  v = 20;
  CHECK(deferred::apply(add, t) == 25);
}

TEST_CASE("apply with empty tuple", "[apply]")
{
  auto t   = std::make_tuple();
  auto res = deferred::apply([] { return 42; }, t);
  CHECK(res == 42);
}

TEST_CASE("apply constexpr", "[apply]")
{
  constexpr auto t   = std::make_tuple(deferred::constant(1), deferred::constant(2));
  constexpr auto res = deferred::apply(add, t);
  static_assert(res == 3, "constexpr apply failed");
  CHECK(res == 3);
}
