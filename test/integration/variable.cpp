// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "deferred/deferred.hpp"

TEST_CASE("operation of variable with constant", "[variable-op-constant]")
{
  auto v  = deferred::variable<int>();
  auto ex = v + deferred::constant(10);
  v       = 42;

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 52);
}

TEST_CASE("operation of variable with variable", "[variable-op-variable]")
{
  auto v1 = deferred::variable<int>();
  auto v2 = deferred::variable<double>();
  auto i  = 10;
  auto d  = 23.3;

  auto ex = v1 + v2;
  v1      = i;
  v2      = d;

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == i + d);
}

TEST_CASE("variable from constant", "[variable-from-constant]")
{
  auto i = 10;
  auto c = deferred::constant(i);
  auto v = deferred::variable(c);

  static_assert(std::is_same_v<decltype(v), decltype(deferred::variable(10))>);
  CHECK(v() == i);
}

TEST_CASE("variable from constant expression", "[variable-from-constant-expression]")
{
  auto ex = deferred::constant(10) + deferred::constant(20);
  auto v  = deferred::variable(ex);

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  CHECK(v() == 30);
}

TEST_CASE("variable written by name is read by an expression", "[variable-get]")
{
  auto v  = deferred::variable(1);
  auto ex = v * deferred::constant(2);

  v.get() = 21;
  CHECK(ex() == 42);
  CHECK(v.get() == 21);
}

TEST_CASE("deduced constants and variables compose into expressions", "[ctad-integration]")
{
  deferred::constant two{2};
  deferred::variable v{1};
  deferred::variable total{0};

  auto loop = deferred::while_(v < 4).do_(deferred::seq(total += v * two, ++v));
  loop();
  CHECK(total() == 12);

  // A variable built from an expression holds its value at that moment.
  deferred::variable snapshot{total + two};
  total = 0;
  CHECK(snapshot() == 14);
}

TEST_CASE("a constant of a move-only type is usable in an expression", "[ctad-integration]")
{
  auto present = deferred::constant(std::make_unique<int>(1)) != nullptr;
  CHECK(present() == true);
}

namespace {

// Callable only as an lvalue, so an expression holds it as a value rather than calling it.
struct lvalue_counter
{
  int n = 0;

  int operator()() &
  {
    return ++n;
  }
};

} // namespace

TEST_CASE("an argument callable only as an lvalue is passed as a value", "[ctad-integration]")
{
  lvalue_counter counter;
  auto i  = deferred::variable(2);
  auto ex = deferred::invoke([](lvalue_counter const& c, int x) { return c.n + x; }, counter, i);

  CHECK(ex() == 2);
  CHECK(counter.n == 0);
}
