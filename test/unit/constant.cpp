/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include "deferred/constant.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"

#include <vector>

TEST_CASE("constant from lvalue", "[constant-lvalue]")
{
  auto const i = 4;
  auto c       = deferred::constant(i);

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(
    std::is_same_v<decltype(c)::value_type, std::remove_cv_t<decltype(i)>>);
  CHECK(c() == i);

  SECTION("constexpr")
  {
    constexpr auto c2 = deferred::constant(i);
    static_assert(deferred::is_constant_expression_v<decltype(c2)>);
    static_assert(
      std::is_same_v<decltype(c2)::value_type, std::remove_cv_t<decltype(i)>>);
    static_assert(c2() == 4, "constexpr failed");
    CHECK(c2() == 4);
  }
}

TEST_CASE("constant from prvalue", "[constant-prvalue]")
{
  auto c = deferred::constant(4);

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(
    std::is_same_v<decltype(c)::value_type, std::remove_cv_t<decltype(4)>>);
  CHECK(c() == 4);

  SECTION("constexpr")
  {
    constexpr auto c2 = deferred::constant(4);

    static_assert(deferred::is_constant_expression_v<decltype(c2)>);
    static_assert(
      std::is_same_v<decltype(c2)::value_type, std::remove_cv_t<decltype(4)>>);
    CHECK(c2() == 4);
    static_assert(c2() == 4, "constexpr failed");
  }
}

TEST_CASE("constant from xvalue", "[constant-xvalue]")
{
  std::vector<int> v = {1, 2, 3};
  auto const v2      = v;
  auto c             = deferred::constant(std::move(v));

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(
    std::is_same_v<decltype(c)::value_type, std::remove_cv_t<decltype(v)>>);
  CHECK(c() == v2);
}

TEST_CASE("constant from constant", "[constant-nested-constant]")
{
  constexpr auto c1 = deferred::constant(4);
  constexpr auto c2 = deferred::constant(c1);

  static_assert(std::is_same_v<decltype(c1), decltype(c2)>,
                "copy created nested type");
  static_assert(
    std::is_same_v<decltype(c1)::value_type, decltype(c2)::value_type>);
  static_assert(c2() == 4, "constexpr failed");
  CHECK(c2() == 4);
}

TEST_CASE("constant from lambda", "[constant-lambda]")
{
  auto i = 0;
  auto c = deferred::constant([&i] { return ++i; });

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, decltype(i)>);
  CHECK(i == 1);
  CHECK(c() == 1);
  CHECK(c() == 1);
}

TEST_CASE("constant from mutable lambda", "[constant-mutable-lambda]")
{
  auto i = 0;
  auto c = deferred::constant([i = i]() mutable { return ++i; });

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, decltype(i)>);
  CHECK(c() == 1);
  CHECK(c() == 1);
  CHECK(i == 0);
}

TEST_CASE("constant from nested lambda", "[constant-nested-lambda]")
{
  auto c = deferred::constant([] { return [] { return 10; }; });

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, decltype(10)>);
  CHECK(c() == 10);
}

TEST_CASE("constant from nested lambda constant",
          "[constant-nested-lambda-constant]")
{
  auto c = deferred::constant([] { return deferred::constant(10); });

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, decltype(10)>);
  CHECK(c() == 10);
}

namespace {

int function()
{
  return 10;
}

} // namespace

TEST_CASE("constant from function", "[constant-from-function]")
{
  auto c = deferred::constant(&function);

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, decltype(function())>);
  CHECK(c() == 10);
}

namespace {

constexpr int function2()
{
  return 10;
}

} // namespace

TEST_CASE("constant from constexpr function",
          "[constant-from-constexpr-function]")
{
  constexpr auto c = deferred::constant(&function2);

  static_assert(deferred::is_constant_expression_v<decltype(c)>);
  static_assert(std::is_same_v<decltype(c)::value_type, decltype(function2())>);
  static_assert(c() == 10, "constexpr failed");
  CHECK(c() == 10);
}
