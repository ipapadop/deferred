/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

#if 0
TEST_CASE("count to n", "[while-variable]")
{
  auto n = deferred::variable<int>();
  n      = 10;

  auto count = 0;
  auto ex    = deferred::while_(n != 0, --n);
  ex();
  CHECK(count == 10);
}
#endif

TEST_CASE("while with nested deferred", "[while-nested-deferred]")
{
  auto n = deferred::variable<int>();
  n      = 10;

  auto count = 0;
  auto ex    = deferred::while_([i = 0, &n]() mutable { return i++ != n; },
                                [&count] { ++count; });
  ex();
  CHECK(count == 10);
}
