// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <valarray>
#include <vector>

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

TEST_CASE("logical operators evaluate each operand once, left to right", "[logical]")
{
  std::vector<char> order;
  auto operand = [&order](char name, bool value) {
    return deferred::invoke([&order, name, value] {
      order.push_back(name);
      return value;
    });
  };

  auto conjunction = operand('a', true) && operand('b', true);
  auto disjunction = operand('c', false) || operand('d', false);

  // Building the expressions evaluates nothing.
  CHECK(order.empty());

  CHECK(conjunction() == true);
  CHECK(order == std::vector{'a', 'b'});

  order.clear();
  CHECK(disjunction() == false);
  CHECK(order == std::vector{'c', 'd'});
}

TEST_CASE("the left operand is evaluated once when it decides the result", "[logical]")
{
  auto evaluated = 0;
  auto left      = [&evaluated](bool value) {
    return deferred::invoke([&evaluated, value] {
      ++evaluated;
      return value;
    });
  };

  CHECK((left(false) && deferred::constant(true))() == false);
  CHECK(evaluated == 1);

  CHECK((left(true) || deferred::constant(false))() == true);
  CHECK(evaluated == 2);
}

TEST_CASE("const and mutable logical expressions agree", "[logical]")
{
  auto evaluated = 0;
  auto right     = deferred::invoke([&evaluated] {
    ++evaluated;
    return true;
  });

  auto left = deferred::variable(false);

  auto ex              = left || right;
  auto const& const_ex = ex;

  static_assert(std::is_same_v<decltype(const_ex()), bool>);
  CHECK(ex() == true);
  CHECK(const_ex() == true);
  CHECK(evaluated == 2);

  auto skipping              = left && right;
  auto const& const_skipping = skipping;

  CHECK(skipping() == false);
  CHECK(const_skipping() == false);
  CHECK(evaluated == 2);
}

TEST_CASE("logical operators evaluate at compile time", "[logical]")
{
  static_assert((deferred::constant(true) && deferred::constant(true))() == true);
  static_assert((deferred::constant(true) && deferred::constant(false))() == false);
  static_assert((deferred::constant(false) || deferred::constant(true))() == true);
  static_assert((deferred::constant(false) || deferred::constant(false))() == false);

  // Evaluating the right operand would not be a constant expression, so these compile
  // only because it is skipped.
  static_assert((deferred::constant(false) && deferred::invoke([]() -> bool { throw 0; }))()
                == false);
  static_assert((deferred::constant(true) || deferred::invoke([]() -> bool { throw 0; }))()
                == true);
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

TEST_CASE("a logical expression exposes its result type", "[logical]")
{
  auto ex = deferred::constant(true) && deferred::constant(false);

  static_assert(std::is_same_v<decltype(ex)::result_type, bool>);
  static_assert(std::is_same_v<decltype(ex()), decltype(ex)::result_type>);
}

namespace {

/// @brief A three-valued boolean: it converts to bool and also overloads && and ||.
struct tribool
{
  int state;

  constexpr explicit operator bool() const noexcept
  {
    return state == 1;
  }
};

struct tribool_result
{
  int state;
};

constexpr tribool_result operator&&(tribool left, tribool right) noexcept
{
  return {left.state < right.state ? left.state : right.state};
}

constexpr tribool_result operator||(tribool left, tribool right) noexcept
{
  return {left.state > right.state ? left.state : right.state};
}

/// @brief Overloads && as a member.
struct member_and
{
  constexpr explicit operator bool() const noexcept
  {
    return true;
  }

  constexpr int operator&&(member_and) const noexcept
  {
    return 7;
  }
};

} // namespace

TEST_CASE("a type with its own logical operators keeps them", "[logical]")
{
  auto unknown = deferred::constant(tribool{2});
  auto yes     = deferred::constant(tribool{1});

  // Converting to bool is not enough to short-circuit: the type's operator decides the
  // result, and its type.
  static_assert(std::is_same_v<decltype((unknown && yes)()), tribool_result>);
  static_assert(std::is_same_v<decltype((unknown || yes)()), tribool_result>);
  CHECK((unknown && yes)().state == 1);
  CHECK((unknown || yes)().state == 2);

  auto m = deferred::constant(member_and{});
  static_assert(std::is_same_v<decltype((m && m)()), int>);
  CHECK((m && m)() == 7);
  // Only && is overloaded for this type, so || still short-circuits to bool.
  static_assert(std::is_same_v<decltype((m || m)()), bool>);
}
