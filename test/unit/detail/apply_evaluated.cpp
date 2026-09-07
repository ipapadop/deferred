// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>

#include "deferred/constant.hpp"
#include "deferred/detail/apply_evaluated.hpp"
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

struct nothrow_add
{
  constexpr int operator()(int a, int b) const noexcept
  {
    return a + b;
  }
};

struct throwing_add
{
  constexpr int operator()(int a, int b) const
  {
    return a + b;
  }
};

// A deferred node that records how many times it is evaluated.
struct counting_node
{
  using subexpression_types = std::tuple<>;

  int* calls;

  constexpr int operator()() const noexcept
  {
    ++*calls;
    return 1;
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

TEST_CASE("apply_evaluated with constants", "[apply-evaluated]")
{
  SECTION("basic usage")
  {
    auto t   = std::make_tuple(deferred::constant(1), deferred::constant(2));
    auto res = deferred::detail::apply_evaluated(add, t);
    CHECK(res == 3);
  }

  SECTION("function object")
  {
    auto t   = std::make_tuple(deferred::constant(3), deferred::constant(4));
    auto res = deferred::detail::apply_evaluated(multiply_t{}, t);
    CHECK(res == 12);
  }

  SECTION("lambda")
  {
    auto t   = std::make_tuple(deferred::constant(10), deferred::constant(5));
    auto res = deferred::detail::apply_evaluated([](auto a, auto b) { return a - b; }, t);
    CHECK(res == 5);
  }
}

TEST_CASE("apply_evaluated with variables", "[apply-evaluated]")
{
  auto v1 = deferred::variable<int>(10);
  auto v2 = deferred::variable<int>(20);
  auto t  = std::forward_as_tuple(v1, v2);

  SECTION("initial values")
  {
    CHECK(deferred::detail::apply_evaluated(add, t) == 30);
  }

  SECTION("updated values")
  {
    v1 = 30;
    v2 = 40;
    CHECK(deferred::detail::apply_evaluated(add, t) == 70);
  }
}

TEST_CASE("apply_evaluated with mixed types", "[apply-evaluated]")
{
  auto v = deferred::variable<int>(10);
  auto c = deferred::constant(5);
  auto t = std::forward_as_tuple(v, c);

  CHECK(deferred::detail::apply_evaluated(add, t) == 15);

  v = 20;
  CHECK(deferred::detail::apply_evaluated(add, t) == 25);
}

TEST_CASE("apply_evaluated with empty tuple", "[apply-evaluated]")
{
  auto t   = std::make_tuple();
  auto res = deferred::detail::apply_evaluated([] { return 42; }, t);
  CHECK(res == 42);
}

TEST_CASE("apply_evaluated constexpr", "[apply-evaluated]")
{
  constexpr auto t   = std::make_tuple(deferred::constant(1), deferred::constant(2));
  constexpr auto res = deferred::detail::apply_evaluated(add, t);
  static_assert(res == 3, "constexpr apply failed");
  CHECK(res == 3);
}

TEST_CASE("apply_evaluated member pointers", "[apply-evaluated]")
{
  widget object{40};

  SECTION("const member function through owned object")
  {
    auto arguments = std::make_tuple(deferred::constant(object));
    CHECK(deferred::detail::apply_evaluated(&widget::get, arguments) == 40);
  }

  SECTION("member function through pointer")
  {
    auto arguments = std::make_tuple(deferred::constant(&object), deferred::constant(2));
    CHECK(deferred::detail::apply_evaluated(&widget::add, arguments) == 42);
    CHECK(object.value == 42);
  }

  SECTION("member function through reference wrapper")
  {
    auto arguments = std::make_tuple(deferred::constant(std::ref(object)), deferred::constant(2));
    CHECK(deferred::detail::apply_evaluated(&widget::add, arguments) == 42);
    CHECK(object.value == 42);
  }

  SECTION("const member function through const reference wrapper")
  {
    auto arguments = std::make_tuple(deferred::constant(std::cref(object)));
    CHECK(deferred::detail::apply_evaluated(&widget::get, arguments) == 40);
  }

  SECTION("member data")
  {
    auto arguments = std::make_tuple(deferred::constant(std::ref(object)));
    static_assert(
      std::is_same_v<decltype(deferred::detail::apply_evaluated(&widget::value, arguments)), int&>);

    auto& value = deferred::detail::apply_evaluated(&widget::value, arguments);
    value       = 42;
    CHECK(object.value == 42);
  }
}

TEST_CASE("apply_evaluated preserves forwarding and exception specifications", "[apply-evaluated]")
{
  SECTION("move-only evaluated argument")
  {
    auto arguments = std::make_tuple(deferred::constant(std::make_unique<int>(42)));
    auto result =
      deferred::detail::apply_evaluated([](std::unique_ptr<int> value) { return *value; },
                                        std::move(arguments));
    CHECK(result == 42);
  }

  SECTION("noexcept callable")
  {
    auto arguments = std::make_tuple(deferred::constant(42));
    static_assert(noexcept(deferred::detail::apply_evaluated(no_throw_identity{}, arguments)));
    CHECK(deferred::detail::apply_evaluated(no_throw_identity{}, arguments) == 42);
  }

  SECTION("throwing callable")
  {
    auto arguments = std::make_tuple(deferred::constant(42));
    static_assert(!noexcept(deferred::detail::apply_evaluated(throwing_identity{}, arguments)));
    CHECK(deferred::detail::apply_evaluated(throwing_identity{}, arguments) == 42);
  }
}

TEST_CASE("apply_invoker evaluates each argument before invoking", "[apply-evaluated]")
{
  nothrow_add adder;
  deferred::detail::apply_invoker<nothrow_add&> invoker{adder};

  auto c1 = deferred::constant(1);
  auto c2 = deferred::constant(2);

  CHECK(invoker(c1, c2) == 3);
}

TEST_CASE("apply_invoker evaluates each argument exactly once", "[apply-evaluated]")
{
  int left  = 0;
  int right = 0;
  nothrow_add adder;
  deferred::detail::apply_invoker<nothrow_add&> invoker{adder};

  CHECK(invoker(counting_node{&left}, counting_node{&right}) == 2);
  CHECK(left == 1);
  CHECK(right == 1);
}

TEST_CASE("apply_invoker dispatches member pointers through std::invoke", "[apply-evaluated]")
{
  using member  = int (widget::*)() const noexcept;
  member getter = &widget::get;
  deferred::detail::apply_invoker<member&> invoker{getter};

  auto c = deferred::constant(widget{40});

  CHECK(invoker(c) == 40);
}

TEST_CASE("apply_invoker propagates the callable's noexcept", "[apply-evaluated]")
{
  nothrow_add nothrow_adder;
  throwing_add throwing_adder;
  deferred::detail::apply_invoker<nothrow_add&> nothrow_invoker{nothrow_adder};
  deferred::detail::apply_invoker<throwing_add&> throwing_invoker{throwing_adder};

  auto c1 = deferred::constant(1);
  auto c2 = deferred::constant(2);

  STATIC_CHECK(noexcept(nothrow_invoker(c1, c2)));
  STATIC_CHECK(!noexcept(throwing_invoker(c1, c2)));
}

TEST_CASE("apply_invoker reads a variable's current value", "[apply-evaluated]")
{
  nothrow_add adder;
  deferred::detail::apply_invoker<nothrow_add&> invoker{adder};

  auto v = deferred::variable<int>(10);
  auto c = deferred::constant(5);

  CHECK(invoker(v, c) == 15);
  v = 20;
  CHECK(invoker(v, c) == 25);
}

TEST_CASE("apply_invoker accepts no arguments", "[apply-evaluated]")
{
  auto callable = [] {
    return 42;
  };
  deferred::detail::apply_invoker<decltype(callable)&> invoker{callable};

  CHECK(invoker() == 42);
}
