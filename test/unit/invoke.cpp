/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include <functional>

#include "deferred/constant.hpp"
#include "deferred/invoke.hpp"
#include "deferred/operators.hpp"
#include "deferred/variable.hpp"

namespace {

int get_int()
{
  return 42;
}

int return_int(int i)
{
  return i;
}

constexpr int constexpr_return_int(int i)
{
  return i;
}

struct get_int_t
{
  constexpr int operator()() const noexcept
  {
    return 42;
  }

  constexpr int operator()(int i) const noexcept
  {
    return i;
  }
};

} // namespace

#include "deferred/type_name.hpp"


TEST_CASE("invoke functions", "[invoke-function]")
{
  SECTION("no arguments")
  {
    auto ex = deferred::invoke(get_int);
    CHECK(ex() == get_int());
  }

  SECTION("literal")
  {
    auto ex = deferred::invoke(return_int, 2);
    CHECK(ex() == return_int(2));
  }

  SECTION("constant")
  {
    auto i  = 42;
    auto c  = deferred::constant(i);
    auto ex = deferred::invoke(return_int, c);
    CHECK(ex() == return_int(i));
  }

  SECTION("immediate constant")
  {
    auto i  = 42;
    auto ex = deferred::invoke(return_int, deferred::constant(i));
    CHECK(ex() == return_int(i));
  }

  SECTION("variable")
  {
    auto i  = 42;
    auto v  = deferred::variable<int>();
    auto ex = deferred::invoke(return_int, v);
    v       = i;
    CHECK(ex() == return_int(i));
  }

  SECTION("constexpr")
  {
    constexpr auto ex = deferred::invoke(constexpr_return_int, 42);
    static_assert(ex() == constexpr_return_int(42), "constexpr failed");
    CHECK(ex() == constexpr_return_int(42));
  }
}

TEST_CASE("invoke function objects", "[invoke-function-objects]")
{
  SECTION("no arguments")
  {
    auto ex = deferred::invoke(get_int_t{});
    CHECK(ex() == get_int_t{}());
  }

  SECTION("literal")
  {
    auto ex = deferred::invoke(get_int_t{}, 2);
    CHECK(ex() == get_int_t{}(2));
  }

  SECTION("constant")
  {
    auto i  = 42;
    auto c  = deferred::constant(i);
    auto ex = deferred::invoke(get_int_t{}, c);
    CHECK(ex() == get_int_t{}(i));
  }

  SECTION("immediate constant")
  {
    auto i  = 42;
    auto ex = deferred::invoke(get_int_t{}, deferred::constant(i));
    CHECK(ex() == get_int_t{}(i));
  }

  SECTION("variable")
  {
    auto i  = 42;
    auto v  = deferred::variable<int>();
    auto ex = deferred::invoke(get_int_t{}, v);
    v       = i;
    CHECK(ex() == get_int_t{}(i));
  }

  SECTION("constexpr")
  {
    constexpr auto ex = deferred::invoke(get_int_t{}, 42);
    static_assert(ex() == get_int_t{}(42), "constexpr failed");
    CHECK(ex() == get_int_t{}(42));
  }
}

TEST_CASE("invoke lambda", "[invoke-lambda]")
{
  auto i  = 0;
  auto ex = deferred::invoke([&] {
    ++i;
    return 10;
  });
  CHECK(i == 0);
  CHECK(ex() == 10);
  CHECK(i == 1);
}

TEST_CASE("invoke mutable lambda", "[invoke-mutable-lambda]")
{
  auto ex = deferred::invoke([i = 0]() mutable { return ++i; });
  CHECK(ex() == 1);
  CHECK(ex() == 2);
}

TEST_CASE("invoke expression", "[invoke-expression]")
{
  auto v = deferred::variable<int>();
  auto c = deferred::constant(10);

  auto ex1 = v + c;
  auto ex2 = deferred::invoke(ex1);

  v = 11;
  CHECK(ex1() == 21);
  CHECK(ex2() == 21);
}

TEST_CASE("invoke expressions", "[invoke-expressions]")
{
  auto v = deferred::variable<int>();
  auto c = deferred::constant(10) + deferred::constant(20);

  auto ex1 = v + c;
  auto ex2 = deferred::invoke([](auto x, auto y) { return x + y; }, ex1, v);
  v        = 3;

  CHECK(ex2() == 36);
}

TEST_CASE("invoke with conditionals", "[invoke-conditionals]")
{
  auto cond = deferred::constant(false);
  auto c    = deferred::constant(10) + deferred::constant(20);
  auto v    = deferred::variable<int>();

  auto ex = deferred::invoke(
    [](auto cond, auto x, auto y) { return cond ? x : y; }, cond, c, v);

  v = 10;

  CHECK(ex() == 10);
}

TEST_CASE("invoke with deferred conditionals", "[invoke-deferred-conditionals]")
{
  auto i = 0;
  auto j = 0;
  auto k = 0;

  auto cond = deferred::invoke([&] {
    ++i;
    return false;
  });
  auto res1 = deferred::invoke([&] {
    ++j;
    return 10;
  });
  auto res2 = deferred::invoke([&] {
    ++k;
    return 20;
  });
  auto ex   = deferred::invoke(
    [](auto cond, auto x, auto y) { return cond ? x : y; }, cond, res1, res2);

  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
  CHECK(ex() == 20);
  // all branches are evaluated; that's why proper conditionals are needed
  CHECK(i == 1);
  CHECK(j == 1);
  CHECK(k == 1);
}
