/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/variable.hpp"

TEST_CASE("empty variable", "[variable-empty]")
{
  auto v = deferred::variable<int>();
  CHECK(v() == int{});

  SECTION("assignment")
  {
    v = 42;
    CHECK(v() == 42);
  }
}

TEST_CASE("initialized variable", "[variable-init]")
{
  auto v = deferred::variable(42);
  CHECK(v() == 42);
}

#if 0
TEST_CASE("unbound variable", "[variable-no-bind]")
{
  deferred::variable<int>();
  FAIL("Should not be able to create unbound variable");
}
#endif

#if 0
TEST_CASE("duplicate variable", "[variable-duplicate]")
{
  auto v1 = deferred::variable<int>();
  auto v2 = v1;
  v1 = 10;
  FAIL("Should not be able to duplicate variable");
}
#endif
