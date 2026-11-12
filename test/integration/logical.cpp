// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("short circuiting guards a side effect", "[logical-short-circuit]")
{
  auto n     = deferred::variable(0);
  auto bumps = deferred::variable(0);

  // The right operand assigns, so it is observable whether it ran.
  auto ex = n != 0 && (bumps += 1);
  CHECK(ex() == false);
  CHECK(bumps() == 0);

  n = 1;
  CHECK(ex() == true);
  CHECK(bumps() == 1);
}

TEST_CASE("short circuiting guards a loop condition", "[logical-short-circuit]")
{
  auto n     = deferred::variable(3);
  auto ready = deferred::variable(true);
  auto ticks = deferred::variable(0);

  auto ex = deferred::while_(ready && n != 0).do_(deferred::seq(ticks += 1, n -= 1));
  ex();
  CHECK(ticks() == 3);

  ready = false;
  n     = 3;
  ticks = 0;
  ex();
  CHECK(ticks() == 0);
}
