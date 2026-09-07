// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "deferred/constant.hpp"
#include "deferred/invoke.hpp"
#include "deferred/operators.hpp"
#include "deferred/variable.hpp"

TEST_CASE("arithmetic operators", "[arithmetic-operators]")
{
  auto i = 41;
  auto j = 3;

  auto c1 = deferred::constant(i);
  auto c2 = deferred::constant(j);

  static_assert(noexcept(c1 + c2));
  static_assert(noexcept(+c1));
  static_assert(noexcept(c1 << c2));

  SECTION("x+y")
  {
    auto e1 = c1 + c2;
    CHECK(e1() == i + j);

    auto e2 = c2 + c1;
    CHECK(e2() == j + i);
  }

  SECTION("x-y")
  {
    auto e1 = c1 - c2;
    CHECK(e1() == i - j);

    auto e2 = c2 - c1;
    CHECK(e2() == j - i);
  }

  SECTION("+x")
  {
    auto e1 = +c1;
    CHECK(e1() == +i);
  }

  SECTION("-x")
  {
    auto e1 = -c1;
    CHECK(e1() == -i);
  }

  SECTION("x*y")
  {
    auto e1 = c1 * c2;
    CHECK(e1() == i * j);

    auto e2 = c2 * c1;
    CHECK(e2() == j * i);
  }

  SECTION("x/y")
  {
    auto e1 = c1 / c2;
    CHECK(e1() == i / j);

    auto e2 = c2 / c1;
    CHECK(e2() == j / i);
  }

  SECTION("x%y")
  {
    auto e1 = c1 % c2;
    CHECK(e1() == i % j);

    auto e2 = c2 % c1;
    CHECK(e2() == j % i);
  }

  SECTION("++x")
  {
    auto v = deferred::variable(10);
    auto e = ++v;
    CHECK(e() == 11);
    CHECK(v() == 11);
  }

  SECTION("x++")
  {
    auto v = deferred::variable(10);
    auto e = v++;
    CHECK(e() == 10);
    CHECK(v() == 11);
  }

  SECTION("--x")
  {
    auto v = deferred::variable(10);
    auto e = --v;
    CHECK(e() == 9);
    CHECK(v() == 9);
  }

  SECTION("x--")
  {
    auto v = deferred::variable(10);
    auto e = v--;
    CHECK(e() == 10);
    CHECK(v() == 9);
  }
}

TEST_CASE("relational operators", "[relational-operators]")
{
  auto i = 41;
  auto j = 3;

  auto c1 = deferred::constant(i);
  auto c2 = deferred::constant(j);
  auto c3 = deferred::constant(i);

  SECTION("x==y")
  {
    auto e1 = c1 == c2;
    CHECK(e1() == (i == j));

    auto e2 = c2 == c1;
    CHECK(e2() == (j == i));

    auto e3 = c1 == c3;
    CHECK(e3());

    auto e4 = c3 == c1;
    CHECK(e4());
  }

  SECTION("x!=y")
  {
    auto e1 = c1 != c2;
    CHECK(e1() == (i != j));

    auto e2 = c2 != c1;
    CHECK(e2() == (j != i));

    auto e3 = c1 != c3;
    CHECK(!e3());

    auto e4 = c3 != c1;
    CHECK(!e4());
  }

  SECTION("x>y")
  {
    auto e1 = c1 > c2;
    CHECK(e1() == (i > j));

    auto e2 = c2 > c1;
    CHECK(e2() == (j > i));

    auto e3 = c1 > c3;
    CHECK(!e3());

    auto e4 = c3 > c1;
    CHECK(!e4());
  }

  SECTION("x<y")
  {
    auto e1 = c1 < c2;
    CHECK(e1() == (i < j));

    auto e2 = c2 < c1;
    CHECK(e2() == (j < i));

    auto e3 = c1 < c3;
    CHECK(!e3());

    auto e4 = c3 < c1;
    CHECK(!e4());
  }

  SECTION("x>y")
  {
    auto e1 = c1 >= c2;
    CHECK(e1() == (i >= j));

    auto e2 = c2 >= c1;
    CHECK(e2() == (j >= i));

    auto e3 = c1 >= c3;
    CHECK(e3());

    auto e4 = c3 >= c1;
    CHECK(e4());
  }

  SECTION("x<y")
  {
    auto e1 = c1 <= c2;
    CHECK(e1() == (i <= j));

    auto e2 = c2 <= c1;
    CHECK(e2() == (j <= i));

    auto e3 = c1 <= c3;
    CHECK(e3());

    auto e4 = c3 <= c1;
    CHECK(e4());
  }
}

TEST_CASE("logical operators", "[logical-operators]")
{
  auto b1 = true;
  auto b2 = false;
  auto b3 = b1;

  auto cb1 = deferred::constant(b1);
  auto cb2 = deferred::constant(b2);
  auto cb3 = deferred::constant(b3);

  SECTION("x&&y")
  {
    auto e1 = cb1 && cb2;
    CHECK(e1() == (b1 && b2));

    auto e2 = cb2 && cb1;
    CHECK(e2() == (b2 && b1));

    auto e3 = cb1 && cb3;
    CHECK(e3() == (b1 && b3));
  }

  SECTION("x||y")
  {
    auto e1 = cb1 || cb2;
    CHECK(e1() == (b1 || b2));

    auto e2 = cb2 || cb1;
    CHECK(e2() == (b2 || b1));

    auto e3 = cb1 || cb3;
    CHECK(e3() == (b1 || b3));
  }

  SECTION("!x")
  {
    auto e1 = !cb1;
    CHECK(e1() == !b1);

    auto e2 = !cb2;
    CHECK(e2() == !b2);

    auto e3 = !!cb2;
    CHECK(e3() == b2);
  }
}

TEST_CASE("bitwise operators", "[bitwise-operators]")
{
  auto i = 13;
  auto j = 3;

  auto c1 = deferred::constant(i);
  auto c2 = deferred::constant(j);

  SECTION("x&y")
  {
    auto e1 = c1 & c2;
    CHECK(e1() == (i & j));

    auto e2 = c2 & c1;
    CHECK(e2() == (j & i));
  }

  SECTION("x|y")
  {
    auto e1 = c1 | c2;
    CHECK(e1() == (i | j));

    auto e2 = c2 | c1;
    CHECK(e2() == (j | i));
  }

  SECTION("x^y")
  {
    auto e1 = c1 ^ c2;
    CHECK(e1() == (i ^ j));

    auto e2 = c2 ^ c1;
    CHECK(e2() == (j ^ i));
  }

  SECTION("~x")
  {
    auto e1 = ~c1;
    CHECK(e1() == ~i);

    auto e2 = ~c2;
    CHECK(e2() == ~j);

    auto e3 = ~~c1;
    CHECK(e3() == i);
  }

  SECTION("x<<y")
  {
    auto e1 = c1 << c2;
    CHECK(e1() == (i << j));

    auto e2 = c2 << c1;
    CHECK(e2() == (j << i));
  }

  SECTION("x>>y")
  {
    auto e1 = c1 >> c2;
    CHECK(e1() == (i >> j));

    auto e2 = c2 >> c1;
    CHECK(e2() == (j >> i));
  }
}

namespace {

// A type whose ++/--/unary+ return a reference to *this, as is conventional.
struct counter
{
  int value;

  constexpr counter& operator++()
  {
    ++value;
    return *this;
  }

  constexpr counter& operator--()
  {
    --value;
    return *this;
  }

  constexpr counter const& operator+() const
  {
    return *this;
  }
};

} // namespace

TEST_CASE("increment and decrement operators return by value", "[operators-return-by-value]")
{
  // The operand yields a prvalue, so a reference result would dangle once the
  // enclosing evaluation returns. These must decay to a value.
  auto source = deferred::invoke([] { return counter{1}; });

  STATIC_CHECK(!std::is_reference_v<decltype((++source)())>);
  STATIC_CHECK(!std::is_reference_v<decltype((--source)())>);
  STATIC_CHECK(!std::is_reference_v<decltype((+source)())>);

  STATIC_CHECK(std::is_same_v<decltype((++source)()), counter>);

  CHECK((++source)().value == 2);
  CHECK((--source)().value == 0);
  CHECK((+source)().value == 1);
}

TEST_CASE("increment through a variable still updates the original", "[operators-return-by-value]")
{
  auto v  = deferred::variable<int>(1);
  auto ex = ++v;

  CHECK(ex() == 2);
  CHECK(v() == 2);
  STATIC_CHECK(!std::is_reference_v<decltype(ex())>);
}
