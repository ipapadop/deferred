// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <valarray>

#include "deferred/constant.hpp"
#include "deferred/invoke.hpp"
#include "deferred/logical.hpp"
#include "deferred/operators.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"
#include "deferred/variable.hpp"

namespace {

struct throwing_boolean
{
  explicit operator bool() const noexcept(false)
  {
    return false;
  }
};

} // namespace

TEST_CASE("and does not evaluate its right operand when the left decides", "[logical]")
{
  auto evaluated = 0;
  auto right     = deferred::invoke([&evaluated] {
    ++evaluated;
    return true;
  });

  CHECK((deferred::constant(false) && right)() == false);
  CHECK(evaluated == 0);

  CHECK((deferred::constant(true) && right)() == true);
  CHECK(evaluated == 1);
}

TEST_CASE("or does not evaluate its right operand when the left decides", "[logical]")
{
  auto evaluated = 0;
  auto right     = deferred::invoke([&evaluated] {
    ++evaluated;
    return false;
  });

  CHECK((deferred::constant(true) || right)() == true);
  CHECK(evaluated == 0);

  CHECK((deferred::constant(false) || right)() == false);
  CHECK(evaluated == 1);
}

TEST_CASE("logical operators yield bool", "[logical]")
{
  auto ex = deferred::constant(1) && deferred::constant(2);

  static_assert(std::is_same_v<decltype(ex()), bool>);
  CHECK(ex() == true);
}


TEST_CASE("logical operators are constant expressions when their operands are", "[logical]")
{
  auto ex = deferred::constant(true) && deferred::constant(false);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(noexcept(ex()));

  auto v = deferred::variable(true);
  static_assert(!deferred::is_constant_expression_v<decltype(v && deferred::constant(true))>);
}

TEST_CASE("logical operators account for a throwing conversion to bool", "[logical]")
{
  auto ex = deferred::constant(throwing_boolean{}) && deferred::constant(true);

  static_assert(!noexcept(ex()));
}

TEST_CASE("operands that do not convert to bool keep the elementwise meaning", "[logical]")
{
  std::valarray<bool> const left{true, false, true};
  std::valarray<bool> const right{true, true, false};

  // std::valarray overloads && elementwise and has no conversion to bool, so it takes
  // the eager overload rather than the short-circuiting one.
  auto ex     = deferred::constant(left) && deferred::constant(right);
  auto result = ex();

  static_assert(!std::is_same_v<decltype(result), bool>);
  REQUIRE(result.size() == 3);
  CHECK(result[0] == true);
  CHECK(result[1] == false);
  CHECK(result[2] == false);
}
