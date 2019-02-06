/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("operators", "[operators]")
{
  auto i = 41;
  auto j = 3;

  auto c1 = deferred::constant(i);
  auto c2 = deferred::constant(j);
  auto c3 = deferred::constant(i);

  SECTION("arithmetic")
  {
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
  }

  SECTION("comparison")
  {
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

  SECTION("logical")
  {}

  SECTION("bitwise")
  {}

  SECTION("compound assignment")
  {}
}


TEST_CASE("operators2", "[operators2]")
{
  auto const i = 41;
  auto const j = 3;
  auto const sum  = i + j;
  auto const diff = i - j;

  auto c1 = deferred::constant(i);
  auto c2 = deferred::constant(j);

  SECTION("expressions with constants")
  {
    SECTION("adding rvalue constants")
    {
      auto e1 = deferred::constant(i) + deferred::constant(j);
      CHECK(e1() == sum);
    }

    SECTION("subtracting rvalue constants")
    {
      auto e1 = deferred::constant(i) - deferred::constant(j);
      CHECK(e1() == diff);
    }

    SECTION("adding lvalue constants")
    {
      auto e1 = c1 + c2;
      CHECK(e1() == sum);
    }

    SECTION("adding rvalue and lvalue constants")
    {
      auto e1 = c1 + deferred::constant(j);
      CHECK(e1() == sum);
    }

    SECTION("adding constant and literal")
    {
      auto e1 = i + c2;
      CHECK(e1() == sum);

      auto e2 = c1 + j;
      CHECK(e2() == sum);
    }
  }

  SECTION("complex expressions")
  {
    auto e1 = c1 + deferred::constant(5);
    auto e2 = e1 + e1;
    auto e3 = e2 - e2;
    CHECK(e3() == 0);
  }
}
