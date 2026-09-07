// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

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

struct widget
{
  int value;

  int get() const noexcept
  {
    return value;
  }

  int add(int increment) noexcept
  {
    value += increment;
    return value;
  }
};

} // namespace

TEST_CASE("apply is the tuple form of invoke", "[apply]")
{
  auto arguments = std::make_tuple(1, 2);

  auto applied = deferred::apply(add, arguments);
  auto invoked = deferred::invoke(add, 1, 2);

  STATIC_CHECK(std::is_same_v<decltype(applied), decltype(invoked)>);
  CHECK(applied() == invoked());
  CHECK(applied() == 3);
}

TEST_CASE("apply builds a deferred expression rather than evaluating", "[apply]")
{
  int calls     = 0;
  auto counting = [&calls](int a, int b) {
    ++calls;
    return a + b;
  };

  auto ex = deferred::apply(counting, std::make_tuple(1, 2));
  CHECK(calls == 0);

  CHECK(ex() == 3);
  CHECK(calls == 1);
}

TEST_CASE("apply is deferred: it observes later variable changes", "[apply]")
{
  auto v  = deferred::variable<int>(10);
  auto ex = deferred::apply(add, std::forward_as_tuple(v, deferred::constant(5)));

  CHECK(ex() == 15);
  v = 20;
  CHECK(ex() == 25);
}

TEST_CASE("apply lifts non-deferred arguments to constants", "[apply]")
{
  auto ex = deferred::apply(add, std::make_tuple(1, 2));

  STATIC_CHECK(deferred::is_deferred_v<decltype(ex)>);
  CHECK(ex() == 3);
}

TEST_CASE("apply with an empty tuple matches a no-argument invoke", "[apply]")
{
  auto applied = deferred::apply([] { return 42; }, std::make_tuple());

  STATIC_CHECK(deferred::is_deferred_v<decltype(applied)>);
  CHECK(applied() == 42);
}

TEST_CASE("apply forwards a deferred callable unchanged", "[apply]")
{
  auto c  = deferred::constant(42);
  auto ex = deferred::apply(c, std::make_tuple());

  STATIC_CHECK(std::is_same_v<decltype(ex), decltype(c)>);
  CHECK(ex() == 42);
}

TEST_CASE("apply defers member pointers", "[apply]")
{
  widget object{40};

  SECTION("const member function")
  {
    auto ex = deferred::apply(&widget::get, std::make_tuple(&object));
    CHECK(ex() == 40);
  }

  SECTION("member function with an argument")
  {
    auto ex = deferred::apply(&widget::add, std::make_tuple(&object, 2));
    CHECK(ex() == 42);
    CHECK(object.value == 42);
  }

  SECTION("member data")
  {
    auto ex = deferred::apply(&widget::value, std::make_tuple(&object));
    STATIC_CHECK(std::is_same_v<decltype(ex()), int&>);
    ex() = 43;
    CHECK(object.value == 43);
  }
}

TEST_CASE("apply is usable in a constant expression", "[apply]")
{
  constexpr auto ex = deferred::apply(add, std::make_tuple(1, 2));
  STATIC_CHECK(ex() == 3);
}
