// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

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

TEST_CASE("make_function_object from function", "[make-function-object-function]")
{
  constexpr auto f = deferred::make_function_object(get_int);

  static_assert(f() == get_int(), "constexpr failed");
  CHECK(f() == get_int());
}

TEST_CASE("make_function_object from function object", "[make-function-object-function-object]")
{
  constexpr auto f = deferred::make_function_object(get_int_t{});

  static_assert(f() == get_int(), "constexpr failed");
  CHECK(f() == get_int());
}
