/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/constant.hpp"
#include "deferred/invoke.hpp"
#include <functional>

namespace
{

int get_int()
{
  return 42;
}

int return_int(int i)
{
  return i;
}

constexpr int constexpr_return_int(int i)
{
  return i;
}

} // namespace

TEST_CASE("invoke", "[invoke]")
{
  SECTION("functions")
  {
    SECTION("no arguments")
    {
      auto ex1 = deferred::invoke(get_int);
      CHECK(ex1() == get_int());
    }

    SECTION("one argument")
    {
      auto ex1 = deferred::invoke(return_int, 42);
      CHECK(ex1() == return_int(42));
    }

    SECTION("constexpr")
    {
      constexpr auto ex1 = deferred::invoke(constexpr_return_int, 42);
      static_assert(ex1() == constexpr_return_int(42), "constexpr failed");
      CHECK(ex1() == constexpr_return_int(42));
    }
  }

  SECTION("function objects")
  {
    SECTION("addition")
    {
      auto v1 = 10;
      auto v2 = 10;
      auto c1 = deferred::constant(v1);
      auto c2 = deferred::constant(v2);

      auto ex1 = deferred::invoke(std::plus<>{}, c1, c2);
      auto ex2 = deferred::invoke(std::plus<>{}, v1, v2);

      CHECK(ex1() == std::plus<>{}(v1, v2));
      CHECK(ex2() == std::plus<>{}(v1, v2));
    }
  }
}
