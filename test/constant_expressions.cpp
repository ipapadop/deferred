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

#include "deferred/type_name.hpp"

// FIXME
TEST_CASE("constant from expression",
          "[constant-from-constant-expression]")
{
  auto i = 10;
  auto ex = deferred::constant(i) * 2;
  auto c = deferred::constant(ex);
  printf("--> %s\n", deferred::type_name<decltype(c)>().c_str());
  //CHECK(c() == 20);
}
#endif

#if 0
// cannot create constant from expression with variable
TEST_CASE("constant from expression with variable",
          "[constant-from-non-constant-expression]")
{
  auto v  = deferred::variable(2);
  auto ex = deferred::constant(4) * v;
  auto c  = deferred::constant(ex);
  CHECK(c() == 8);
}
#endif
