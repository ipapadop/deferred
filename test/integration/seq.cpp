// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("seq gives a loop body more than one statement", "[seq-loop]")
{
  auto n     = deferred::variable(4);
  auto ticks = deferred::variable(0);

  auto ex = deferred::while_(n != 0).do_(deferred::seq(ticks += 1, n -= 1));
  ex();
  CHECK(n() == 0);
  CHECK(ticks() == 4);
}

TEST_CASE("seq gives a for clause more than one statement", "[seq-for]")
{
  auto a     = deferred::variable<int>();
  auto b     = deferred::variable<int>();
  auto total = deferred::variable(0);

  // A complete C-style loop with no lambda in any clause.
  auto ex = deferred::for_(deferred::seq(a = deferred::constant(0), b = deferred::constant(100)),
                           a < 3,
                           deferred::seq(a += 1, b -= 1))
              .do_(total += b);
  ex();
  CHECK(a() == 3);
  CHECK(total() == 100 + 99 + 98);
}

TEST_CASE("seq in a conditional branch", "[seq-conditional]")
{
  auto n    = deferred::variable(0);
  auto hits = deferred::variable(0);

  auto ex = deferred::if_(n < 10).then_(deferred::seq(hits += 1, n += 5)).else_(hits -= 1);

  // The branch yields its last sequenced expression.
  CHECK(ex() == 5);
  CHECK(n() == 5);
  CHECK(hits() == 1);

  CHECK(ex() == 10);
  CHECK(n() == 10);
  CHECK(hits() == 2);

  CHECK(ex() == 1);
  CHECK(n() == 10);
  CHECK(hits() == 1);
}
