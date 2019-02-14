/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/conditional.hpp"

#if 0

TEST_CASE("conditional with literal", "[conditional-literal]")
{
  auto ex = deferred::if_then_else(true, 42, 10);
  CHECK(ex() == 42);
}

#endif

TEST_CASE("conditional with lambda", "[conditional-lambdas]")
{
  auto i = 0;
  auto ex = deferred::if_then_else([] { return true; }, [&] { ++i; return 42; }, [&] { ++i; return 10; });
  CHECK(i == 0);
  CHECK(ex() == 42);
  CHECK(i == 1);
}

#if 0
TEST_CASE("if with literal", "[conditional-lambda]")
{
  auto i = 0;
  auto ex1 = deferred::if_([&i] { i = 1; });
  CHECK(i == 0);
  ex1();
  CHECK(i == 1);
}
#endif
