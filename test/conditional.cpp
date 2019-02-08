/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/conditional.hpp"
#include "deferred/constant.hpp"
#include "deferred/variable.hpp"

TEST_CASE("conditionals", "[conditionals]")
{
  SECTION("if with literal")
  {
    auto ex1 = deferred::if_(true);
    ex1();
    CHECK(true);
  }

  SECTION("if with lambda")
  {
    auto i = 0;
    auto ex1 = deferred::if_([&i] { i = 1; });
    CHECK(i == 0);
    ex1();
    CHECK(i == 1);
  }
}
