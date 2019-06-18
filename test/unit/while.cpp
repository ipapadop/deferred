/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/while.hpp"

TEST_CASE("while with lambda", "[while-lambda]")
{
  auto i  = 0;
  auto j  = 0;
  auto ex = deferred::while_([&i] { return ++i < 10; }, [&j] { j += 2; });

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  ex();
  CHECK(i == 10);
  CHECK(j == 18);
}

TEST_CASE("while with constexpr", "[while-constexpr]")
{
  auto const i      = 0;
  constexpr auto ex = deferred::while_([] { return i != 0; }, [] {});

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert((ex(), i) == 0, "Failed");
  ex();
  CHECK(i == 0);
}
