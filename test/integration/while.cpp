// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("while with body decrement", "[while-body]")
{
  auto n = deferred::variable<int>();
  n      = 10;

  auto ex = deferred::while_(n != 0).do_(--n);
  ex();
  CHECK(n() == 0);
}

TEST_CASE("while with condition decrement", "[while-condition]")
{
  auto n = deferred::variable(10);

  int count = 0;
  auto ex   = deferred::while_(--n != 0).do_([&count] { ++count; });
  ex();
  CHECK(n() == 0);
  CHECK(count == 9);
}

TEST_CASE("while with nested deferred", "[while-nested-deferred]")
{
  auto n = deferred::variable<int>();
  n      = 10;

  auto count = 0;
  auto ex = deferred::while_([i = 0, &n]() mutable { return i++ != n; }).do_([&count] { ++count; });
  ex();
  CHECK(count == 10);
}

TEST_CASE("while with a nested conditional body", "[while-nested-conditional]")
{
  auto n       = deferred::variable<int>();
  auto evens   = 0;
  auto counter = deferred::if_([&n] { return n() % 2 == 0; }).then_([&evens] { ++evens; });
  auto ex      = deferred::while_(n != 0).do_([&] {
    counter();
    n = n() - 1;
  });

  n = 4;
  ex();
  CHECK(n() == 0);
  CHECK(evens == 2);
}
