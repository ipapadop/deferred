/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include <functional>

#include "deferred/make_function_object.hpp"

namespace {

constexpr int get_int()
{
  return 42;
}

struct get_int_t
{
  constexpr int operator()() const noexcept
  {
    return get_int();
  }
};

} // namespace

TEST_CASE("make_function_object from function",
          "[make-function-object-function]")
{
  constexpr auto f = deferred::make_function_object(get_int);

  static_assert(f() == get_int(), "constexpr failed");
  CHECK(f() == get_int());
}

TEST_CASE("make_function_object from function object",
          "[make-function-object-function-object]")
{
  constexpr auto f = deferred::make_function_object(get_int_t{});

  static_assert(f() == get_int(), "constexpr failed");
  CHECK(f() == get_int());
}
