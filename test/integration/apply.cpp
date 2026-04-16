// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("apply with constants", "[apply-constants]")
{
  auto c1 = deferred::constant(10);
  auto c2 = deferred::constant(10);
  auto ex = c1 + c2;

  CHECK(deferred::apply(std::plus<int>{}, std::make_tuple(c1, c2)) == ex());
}
