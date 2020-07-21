/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("while with body decrement", "[while-body]")
{
  auto n = deferred::variable<int>();
  n      = 10;

  auto ex = deferred::while_(n != 0, --n);
  ex();
  CHECK(n() == 0);
}

TEST_CASE("while with condition decrement", "[while-condition]")
{
  auto n = deferred::variable(10);

  int count = 0;
  auto ex   = deferred::while_(--n != 0, [&count] { ++count; });
  ex();
  CHECK(n() == 0);
  CHECK(count == 9);
}

TEST_CASE("while with nested deferred", "[while-nested-deferred]")
{
  auto n = deferred::variable<int>();
  n      = 10;

  auto count = 0;
  auto ex    = deferred::while_([i = 0, &n]() mutable { return i++ != n; }, [&count] { ++count; });
  ex();
  CHECK(count == 10);
}
