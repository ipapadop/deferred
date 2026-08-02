// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>

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

struct widget
{
  int value;

  constexpr int get() const noexcept
  {
    return value;
  }

  constexpr int add(int increment) noexcept
  {
    value += increment;
    return value;
  }
};

struct no_throw_identity
{
  constexpr int operator()(int value) const noexcept
  {
    return value;
  }
};

struct throwing_identity
{
  constexpr int operator()(int value) const
  {
    return value;
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

TEST_CASE("apply member pointers", "[apply]")
{
  widget object{40};

  SECTION("const member function through owned object")
  {
    auto arguments = std::make_tuple(deferred::constant(object));
    CHECK(deferred::apply(&widget::get, arguments) == 40);
  }

  SECTION("member function through pointer")
  {
    auto arguments = std::make_tuple(deferred::constant(&object), deferred::constant(2));
    CHECK(deferred::apply(&widget::add, arguments) == 42);
    CHECK(object.value == 42);
  }

  SECTION("member function through reference wrapper")
  {
    auto arguments = std::make_tuple(deferred::constant(std::ref(object)), deferred::constant(2));
    CHECK(deferred::apply(&widget::add, arguments) == 42);
    CHECK(object.value == 42);
  }

  SECTION("const member function through const reference wrapper")
  {
    auto arguments = std::make_tuple(deferred::constant(std::cref(object)));
    CHECK(deferred::apply(&widget::get, arguments) == 40);
  }

  SECTION("member data")
  {
    auto arguments = std::make_tuple(deferred::constant(std::ref(object)));
    static_assert(std::is_same_v<decltype(deferred::apply(&widget::value, arguments)), int&>);

    auto& value = deferred::apply(&widget::value, arguments);
    value       = 42;
    CHECK(object.value == 42);
  }
}

TEST_CASE("apply preserves forwarding and exception specifications", "[apply]")
{
  SECTION("move-only evaluated argument")
  {
    auto arguments = std::make_tuple(deferred::constant(std::make_unique<int>(42)));
    auto result =
      deferred::apply([](std::unique_ptr<int> value) { return *value; }, std::move(arguments));
    CHECK(result == 42);
  }

  SECTION("noexcept callable")
  {
    auto arguments = std::make_tuple(deferred::constant(42));
    static_assert(noexcept(deferred::apply(no_throw_identity{}, arguments)));
    CHECK(deferred::apply(no_throw_identity{}, arguments) == 42);
  }

  SECTION("throwing callable")
  {
    auto arguments = std::make_tuple(deferred::constant(42));
    static_assert(!noexcept(deferred::apply(throwing_identity{}, arguments)));
    CHECK(deferred::apply(throwing_identity{}, arguments) == 42);
  }
}
