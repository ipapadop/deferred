/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("apply with constants", "[apply-constants]")
{
  auto c1 = deferred::constant(10);
  auto c2 = deferred::constant(10);
  auto ex = c1 + c2;

  CHECK(deferred::apply(std::plus<int>{}, std::make_tuple(c1, c2)) == ex());
}
