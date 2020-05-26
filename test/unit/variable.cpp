/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/variable.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"

TEST_CASE("empty variable", "[variable-empty]")
{
  auto v = deferred::variable<int>();

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int&>);
  CHECK(v() == int{});

  SECTION("assignment")
  {
    v = 42;
    CHECK(v() == 42);
  }
}

TEST_CASE("initialized variable", "[variable-init]")
{
  constexpr auto v = deferred::variable(42);

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int const&>);
  CHECK(v() == 42);
}

TEST_CASE("constexpr variable", "[variable-constexpr]")
{
  constexpr auto v = deferred::variable(42);

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int const&>);
  static_assert(v() == 42);
}

TEST_CASE("variable from lambda", "[variable-from-lambda]")
{
  int i = 0;
  auto v = deferred::variable([&] {
    ++i;
    return 10;
  });

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int&>);
  CHECK(i == 1);
  CHECK(v() == 10);
}

TEST_CASE("variable from mutable lambda", "[variable-from-mutable-lambda]")
{
  auto v = deferred::variable([i = 0]() mutable { return ++i; });

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int&>);
  CHECK(v() == 1);
  CHECK(v() == 1);
}

namespace {

int function()
{
  return 10;
}

} // namespace

TEST_CASE("variable from function", "[variable-from-function]")
{
  auto v = deferred::variable(&function);

  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int&>);
  CHECK(v() == 10);
}

#if 0
TEST_CASE("unbound variable", "[variable-no-bind]")
{
  deferred::variable<int>();
  FAIL("Should not be able to create unbound variable");
}
#endif

#if 0
TEST_CASE("duplicate variable", "[variable-duplicate]")
{
  auto v1 = deferred::variable<int>();
  auto v2 = v1;
  v1 = 10;
  FAIL("Should not be able to duplicate variable");
}
#endif
