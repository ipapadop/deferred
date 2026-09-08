// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("deferred assignment composes with operators", "[assign-operators]")
{
  auto v = deferred::variable<int>();
  auto n = deferred::variable(4);

  auto ex = (v = n * 2 + 1);
  ex();
  CHECK(v() == 9);

  n = 10;
  ex();
  CHECK(v() == 21);
}

TEST_CASE("deferred assignment as a loop body", "[assign-loop]")
{
  auto n  = deferred::variable(5);
  auto ex = deferred::while_(n != 0).do_(n = n - 1);

  ex();
  CHECK(n() == 0);
}

TEST_CASE("deferred assignment inside a conditional branch", "[assign-conditional]")
{
  auto n    = deferred::variable(0);
  auto flag = deferred::variable(true);
  auto ex   = deferred::if_(flag).then_(n = deferred::constant(1)).else_(n = deferred::constant(2));

  // The branch yields the assigned value, as a built-in assignment would.
  CHECK(ex() == 1);
  CHECK(n() == 1);

  flag = false;
  CHECK(ex() == 2);
  CHECK(n() == 2);
}

TEST_CASE("compound assignment as a loop step and body", "[assign-compound]")
{
  auto i   = deferred::variable<int>();
  auto sum = deferred::variable(0);

  // No lambda in any clause.
  auto ex = deferred::for_(i = deferred::constant(0), i < 5, i += 1).do_(sum += i);
  ex();
  CHECK(i() == 5);
  CHECK(sum() == 0 + 1 + 2 + 3 + 4);
}

TEST_CASE("compound assignment composes with operators", "[assign-compound]")
{
  auto v = deferred::variable(1);
  auto n = deferred::variable(3);

  auto ex = (v *= n + 1);
  ex();
  CHECK(v() == 4);

  n = 2;
  ex();
  CHECK(v() == 12);
}
