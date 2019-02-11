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
TEST_CASE("variable from constant", "[variable-from-constant]")
{
  auto i = 10;
  auto c = deferred::constant(i);
  auto v = deferred::variable(c);
  printf("--> %s\n", deferred::type_name<decltype(v)>().c_str());
  printf("%d\n", v());
  CHECK(v() == 30);
}

#endif

#if 0
#include "deferred/type_name.hpp"

// FIXME
TEST_CASE("variable from expression", "[variable-from-expression]")
{
  auto ex = deferred::constant(10) + deferred::constant(20);
  auto v  = deferred::variable(ex);
  CHECK(v() == 30);
}
#endif
