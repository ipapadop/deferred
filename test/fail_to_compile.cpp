/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

// all the tests in this file should fail to compile

#if 0
TEST_CASE("constant from expression", "[constant-from-expression]")
{
  auto ex = deferred::constant(4) + 4;
  auto c2 = deferred::constant(ex);
  CHECK(c2() == 8);
}
#endif

#if 0
TEST_CASE("uncaptured variable", "[variable-no-capture]")
{
  auto ex = deferred::variable<int>() + deferred::constant(10);
  CHECK(ex() == 10);
}
#endif
