/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/constant.hpp"

#include <vector>

TEST_CASE("constant from lvalue", "[constant_lvalue]")
{
  auto i = 4;
  auto c = deferred::constant(i);
  CHECK(c() == i);
}

TEST_CASE("constant from prvalue", "[constant_prvalue]")
{
  auto c = deferred::constant(4);
  CHECK(c() == 4);

  SECTION("constexpr")
  {
    constexpr auto c2 = deferred::constant(4);
    CHECK(c2() == 4);
    static_assert(c2() == 4, "constexpr failed");
  }
}

TEST_CASE("constant from xvalue", "[constant_xvalue]")
{
  std::vector<int> v = { 1, 2, 3 };
  auto const v2 = v;
  auto c = deferred::constant(std::move(v));
  CHECK(c() == v2);
}

TEST_CASE("unwrapping constant", "[constant_unwrap]")
{
  auto i = 4;
  auto c1 = deferred::constant(i);
  auto c2 = deferred::constant(c1);
  static_assert(std::is_same_v<decltype(c1), decltype(c2)>, "copy created nested type");
  CHECK(c2() == i);
}
