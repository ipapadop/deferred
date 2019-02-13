/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("if with constant", "[conditional-constant]")
{
  auto c = deferred::constant(true);
  auto ex = deferred::if_(c);
  ex();
  CHECK(true);
}

TEST_CASE("if with variable", "[conditional-variable]")
{
  auto c = deferred::variable(true);
  auto ex = deferred::if_(c);
  ex();
  CHECK(true);
}
