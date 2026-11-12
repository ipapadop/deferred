// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("do-while with body decrement", "[do-while-body]")
{
  auto n = deferred::variable<int>();
  n      = 10;

  auto ex = deferred::do_(--n).while_(n != 0);
  ex();
  CHECK(n() == 0);
}

TEST_CASE("do-while runs its body before checking the condition", "[do-while-body]")
{
  auto n = deferred::variable(0);

  auto ex = deferred::do_(--n).while_(n > 0);
  ex();
  CHECK(n() == -1);
}

TEST_CASE("do-while with condition decrement", "[do-while-condition]")
{
  auto n = deferred::variable(10);

  int count = 0;
  auto ex   = deferred::do_([&count] { ++count; }).while_(--n != 0);
  ex();
  CHECK(n() == 0);
  CHECK(count == 10);
}

TEST_CASE("do-while with a nested conditional body", "[do-while-nested-conditional]")
{
  auto n       = deferred::variable<int>();
  auto evens   = 0;
  auto counter = deferred::if_([&n] { return n() % 2 == 0; }).then_([&evens] { ++evens; });
  auto ex      = deferred::do_([&] {
              counter();
              n = n() - 1;
                 }).while_(n != 0);

  n = 4;
  ex();
  CHECK(n() == 0);
  CHECK(evens == 2);
}
