/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/constant.hpp"
#include "deferred/operators.hpp"

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

  SECTION("bitwise")
  {
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

  SECTION("compound assignment")
  {}
}
