// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "deferred/constant.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"

namespace {

struct throwing_move
{
  throwing_move()                     = default;
  throwing_move(throwing_move const&) = delete;
  throwing_move(throwing_move&&) noexcept(false);
};

} // namespace

static_assert(!noexcept(std::declval<deferred::constant<throwing_move>&&>()()));
static_assert(!noexcept(std::declval<deferred::constant<throwing_move>&&>().get()));

TEST_CASE("constant from array decays without throwing", "[constant-array-decay]")
{
  static_assert(noexcept(deferred::evaluate("hello")));
  static_assert(noexcept(deferred::constant("hello")));

  auto c = deferred::constant("hello");

  static_assert(std::is_same_v<decltype(c)::value_type, char const*>);
  CHECK(std::string(c()) == "hello");
}

TEST_CASE("constant from lvalue", "[constant-lvalue]")
{
  auto const i = 4;
  auto c       = deferred::constant(i);

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, int>);
  static_assert(std::is_same_v<decltype(c()), int const&>);
  CHECK(c() == i);

  SECTION("constexpr")
  {
    constexpr auto c2 = deferred::constant(i);
    static_assert(deferred::is_constant_expression_v<decltype(c2)>);
    static_assert(std::is_same_v<decltype(c2)::value_type, int>);
    static_assert(std::is_same_v<decltype(c2()), int const&>);
    static_assert(c2() == 4, "constexpr failed");
    CHECK(c2() == 4);
  }
}

TEST_CASE("constant from prvalue", "[constant-prvalue]")
{
  auto c = deferred::constant(4);

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, int>);
  static_assert(std::is_same_v<decltype(c()), int const&>);
  CHECK(c() == 4);

  SECTION("constexpr")
  {
    constexpr auto c2 = deferred::constant(4);

    static_assert(deferred::is_constant_expression_v<decltype(c2)>);
    static_assert(std::is_same_v<decltype(c2)::value_type, int>);
    static_assert(std::is_same_v<decltype(c2()), int const&>);
    CHECK(c2() == 4);
    static_assert(c2() == 4, "constexpr failed");
  }
}

TEST_CASE("constant from xvalue", "[constant-xvalue]")
{
  std::vector<int> v = {1, 2, 3};
  auto const v2      = v;
  auto c             = deferred::constant(std::move(v));

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, std::vector<int>>);
  static_assert(std::is_same_v<decltype(c()), std::vector<int> const&>);
  CHECK(c() == v2);
}

TEST_CASE("constant from constant", "[constant-nested-constant]")
{
  constexpr auto c1 = deferred::constant(4);
  constexpr auto c2 = deferred::constant(c1);

  static_assert(std::is_same_v<decltype(c1), decltype(c2)>, "copy created nested type");
  static_assert(std::is_same_v<decltype(c1)::value_type, int>);
  static_assert(std::is_same_v<decltype(c2)::value_type, int>);
  static_assert(std::is_same_v<decltype(c1()), int const&>);
  static_assert(std::is_same_v<decltype(c2()), int const&>);
  static_assert(c2() == 4, "constexpr failed");
  CHECK(c2() == 4);
}

TEST_CASE("constant from lambda", "[constant-lambda]")
{
  int i  = 0;
  auto c = deferred::constant([&i] { return ++i; });

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, int>);
  static_assert(std::is_same_v<decltype(c()), int const&>);
  CHECK(i == 1);
  CHECK(c() == 1);
  CHECK(c() == 1);
}

TEST_CASE("constant from mutable lambda", "[constant-mutable-lambda]")
{
  unsigned int i = 0;
  auto c         = deferred::constant([i = i]() mutable { return ++i; });

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, unsigned int>);
  static_assert(std::is_same_v<decltype(c()), unsigned int const&>);
  CHECK(c() == 1);
  CHECK(c() == 1);
  CHECK(i == 0);
}

TEST_CASE("constant from nested lambda", "[constant-nested-lambda]")
{
  auto c = deferred::constant([] {
    return [] {
      return 10;
    };
  });

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, int>);
  static_assert(std::is_same_v<decltype(c()), int const&>);
  CHECK(c() == 10);
}

TEST_CASE("constant from nested lambda constant", "[constant-nested-lambda-constant]")
{
  auto c = deferred::constant([] { return deferred::constant(10); });

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, int>);
  static_assert(std::is_same_v<decltype(c()), int const&>);
  CHECK(c() == 10);
}

namespace {

int function()
{
  return 10;
}

} // namespace

TEST_CASE("constant from function", "[constant-from-function]")
{
  auto c = deferred::constant(&function);

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, int>);
  static_assert(std::is_same_v<decltype(c()), int const&>);
  CHECK(c() == 10);
}

namespace {

constexpr int function2()
{
  return 10;
}

} // namespace

TEST_CASE("constant from constexpr function", "[constant-from-constexpr-function]")
{
  constexpr auto c = deferred::constant(&function2);

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, int>);
  static_assert(std::is_same_v<decltype(c()), int const&>);
  static_assert(c() == 10, "constexpr failed");
  CHECK(c() == 10);
}

TEST_CASE("constant named access", "[constant-get]")
{
  constexpr auto c = deferred::constant(42);

  static_assert(std::is_same_v<decltype(c.get()), decltype(c())>);
  static_assert(std::is_same_v<decltype(c.get()), int const&>);
  static_assert(std::is_same_v<decltype(std::move(c).get()), int const&>);
  static_assert(noexcept(c.get()));
  static_assert(c.get() == 42);
  CHECK(&c.get() == &c());

  SECTION("move access")
  {
    auto s = deferred::constant(std::string("a string long enough to be allocated"));

    static_assert(std::is_same_v<decltype(std::move(s).get()), std::string>);
    static_assert(std::is_same_v<decltype(std::move(s).get()), decltype(std::move(s)())>);
    static_assert(noexcept(std::move(s).get()));

    auto moved = std::move(s).get();
    CHECK(moved == "a string long enough to be allocated");
  }
}

namespace {

int free_function()
{
  return 3;
}

} // namespace

TEST_CASE("constant deduces its type from a value", "[constant-ctad]")
{
  deferred::constant c{42};
  static_assert(std::is_same_v<decltype(c), deferred::constant<int>>);
  static_assert(std::is_same_v<decltype(deferred::constant(42)), deferred::constant<int>>);
  CHECK(c() == 42);

  // Neither a reference nor a cv-qualifier is retained: the constant owns its value.
  int i         = 1;
  int const ci  = 2;
  int& r        = i;
  int const& cr = ci;
  static_assert(std::is_same_v<decltype(deferred::constant{i}), deferred::constant<int>>);
  static_assert(std::is_same_v<decltype(deferred::constant{ci}), deferred::constant<int>>);
  static_assert(std::is_same_v<decltype(deferred::constant{r}), deferred::constant<int>>);
  static_assert(std::is_same_v<decltype(deferred::constant{cr}), deferred::constant<int>>);
  static_assert(
    std::is_same_v<decltype(deferred::constant{std::move(i)}), deferred::constant<int>>);

  deferred::constant owned{i};
  i = 100;
  CHECK(owned() == 1);
}

TEST_CASE("constant deduces a decayed type from an array", "[constant-ctad]")
{
  int numbers[] = {1, 2, 3};

  static_assert(
    std::is_same_v<decltype(deferred::constant{"text"}), deferred::constant<char const*>>);
  static_assert(std::is_same_v<decltype(deferred::constant{numbers}), deferred::constant<int*>>);
}

TEST_CASE("constant deduces the result type of a callable and stores the result", "[constant-ctad]")
{
  auto calls = 0;
  auto count = [&calls] {
    return ++calls;
  };

  deferred::constant c{count};
  static_assert(std::is_same_v<decltype(c), deferred::constant<int>>);

  // The callable ran once, when the constant was built.
  CHECK(calls == 1);
  CHECK(c() == 1);
  CHECK(c() == 1);

  static_assert(
    std::is_same_v<decltype(deferred::constant{free_function}), deferred::constant<int>>);
  static_assert(
    std::is_same_v<decltype(deferred::constant{&free_function}), deferred::constant<int>>);

  // Evaluation repeats until the result is not callable.
  auto nested = [] {
    return [] {
      return 2.5;
    };
  };
  static_assert(std::is_same_v<decltype(deferred::constant{nested}), deferred::constant<double>>);

  // A const callable is evaluated too, not stored.
  auto const fixed = [] {
    return 7;
  };
  static_assert(std::is_same_v<decltype(deferred::constant{fixed}), deferred::constant<int>>);
}

TEST_CASE("constant with a named type converts its argument", "[constant-ctad]")
{
  deferred::constant<long> c{1};
  static_assert(std::is_same_v<decltype(c()), long const&>);
  CHECK(c() == 1L);

  // A callable argument is still evaluated first.
  deferred::constant<long> from_callable{free_function};
  CHECK(from_callable() == 3L);

  static_assert(!std::is_constructible_v<deferred::constant<int>, std::string>);
}

TEST_CASE("constant from a constant is a copy", "[constant-ctad]")
{
  deferred::constant c{42};
  deferred::constant copy{c};
  deferred::constant from_const{std::as_const(c)};

  static_assert(std::is_same_v<decltype(copy), deferred::constant<int>>);
  static_assert(std::is_same_v<decltype(from_const), deferred::constant<int>>);
  CHECK(copy() == 42);
  CHECK(from_const() == 42);
}
