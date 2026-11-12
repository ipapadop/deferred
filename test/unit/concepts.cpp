// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>
#include <vector>

#include "deferred/deferred.hpp"

namespace {

struct not_a_condition
{ };

struct explicitly_boolean
{
  explicit operator bool() const noexcept
  {
    return true;
  }
};

template<typename T>
concept usable_in_if = requires(T t) { deferred::if_(t); };

template<typename T>
concept usable_in_else_if = requires(T t) { deferred::if_(true).then_(1).else_if_(t); };

template<typename T>
concept usable_in_while = requires(T t) { deferred::while_(t); };

template<typename T>
concept usable_in_do_while = requires(T t) { deferred::do_(1).while_(t); };

template<typename T>
concept usable_in_for = requires(T t, deferred::variable_<int>& i) { deferred::for_(++i, t, ++i); };

template<typename Label, typename Condition>
concept usable_as_first_case =
  requires(Label label, Condition condition) { deferred::switch_(condition).case_(label); };

template<typename Label, typename Condition>
concept usable_as_later_case = requires(Label label, Condition condition) {
  deferred::switch_(condition).case_(condition).then_(1).case_(label);
};

} // namespace

TEST_CASE("expression_yielding checks the result of calling the expression", "[concepts]")
{
  using deferred::expression_yielding;

  using C = decltype(deferred::constant(42));
  using V = deferred::variable_<int>;

  static_assert(expression_yielding<C, int>);
  static_assert(expression_yielding<C, int const&>);
  static_assert(expression_yielding<C&, long>);
  static_assert(!expression_yielding<C, std::string>);

  // A reference result is checked as a reference.
  static_assert(expression_yielding<V, int&>);
  static_assert(!expression_yielding<V const, int&>);
  static_assert(expression_yielding<V const&, int const&>);
  static_assert(!expression_yielding<C, int&>);

  auto v   = deferred::variable(1);
  auto sum = v + deferred::constant(2);
  static_assert(expression_yielding<decltype(sum), int>);
  static_assert(!expression_yielding<decltype(sum), int&>);

  // Something that is not a deferred expression never satisfies it.
  static_assert(!expression_yielding<int, int>);
  static_assert(!expression_yielding<int (*)(), int>);
  static_assert(!expression_yielding<decltype(deferred::if_(v > 0)), int>);
}

TEST_CASE("a void expression yields void and nothing else", "[concepts]")
{
  auto i    = deferred::variable(0);
  auto loop = deferred::while_(i < 3).do_(++i);

  static_assert(!deferred::expression_yielding<decltype(loop), int>);
  static_assert(deferred::expression_yielding<decltype(loop), void>);
  static_assert(!deferred::expression_yielding<decltype(i + 1), void>);
}

TEST_CASE("lvalue_expression is satisfied when the result is an lvalue", "[concepts]")
{
  using deferred::lvalue_expression;

  auto v = deferred::variable(std::vector{1, 2, 3});
  auto c = deferred::constant(1);

  static_assert(lvalue_expression<decltype(v)&>);
  static_assert(lvalue_expression<decltype(c)&>);
  static_assert(lvalue_expression<decltype(deferred::subscript(v, 0))>);

  auto i = deferred::variable(0);
  static_assert(lvalue_expression<decltype(++i)>);
  static_assert(lvalue_expression<decltype(i += 1)>);
  static_assert(!lvalue_expression<decltype(i + 1)>);
  static_assert(!lvalue_expression<decltype(i++)>);
  static_assert(!lvalue_expression<decltype(deferred::invoke([] { return 1; }))>);

  static_assert(!lvalue_expression<int>);
  static_assert(!lvalue_expression<int&>);
}

TEST_CASE("condition is satisfied by anything whose result converts to bool", "[concepts]")
{
  using deferred::condition;

  auto i = deferred::variable(0);

  static_assert(condition<bool>);
  static_assert(condition<int>);
  static_assert(condition<int*>);
  static_assert(condition<decltype(i)&>);
  static_assert(condition<decltype(i < 3)>);
  static_assert(condition<decltype(i < 3 && i > 0)>);
  static_assert(condition<decltype(deferred::constant(std::make_shared<int>(1)))>);

  // An explicit conversion is enough, as for the condition of a built-in if.
  static_assert(condition<explicitly_boolean>);
  static_assert(condition<decltype(deferred::constant(explicitly_boolean{}))>);

  // A callable is made into an expression, so its result is what is tested.
  static_assert(condition<bool (*)()>);
  static_assert(!condition<std::string (*)()>);

  static_assert(!condition<std::string>);
  static_assert(!condition<not_a_condition>);
  static_assert(!condition<decltype(deferred::constant(std::string{}))>);
  static_assert(!condition<decltype(deferred::while_(i < 3).do_(++i))>);
}

TEST_CASE("the constructs that take a condition reject anything else", "[concepts]")
{
  using good = deferred::constant_<bool>;
  using bad  = deferred::constant_<std::string>;

  static_assert(usable_in_if<good>);
  static_assert(usable_in_if<bool>);
  static_assert(!usable_in_if<bad>);
  static_assert(!usable_in_if<not_a_condition>);

  static_assert(usable_in_else_if<good>);
  static_assert(!usable_in_else_if<bad>);

  static_assert(usable_in_while<good>);
  static_assert(!usable_in_while<bad>);

  static_assert(usable_in_do_while<good>);
  static_assert(!usable_in_do_while<bad>);

  static_assert(usable_in_for<good>);
  static_assert(!usable_in_for<bad>);
}

TEST_CASE("case_label_for is satisfied by a label comparable to the condition", "[concepts]")
{
  using deferred::case_label_for;

  using I = deferred::variable_<int>&;
  using S = deferred::variable_<std::string>&;

  static_assert(case_label_for<int, I>);
  static_assert(case_label_for<long, I>);
  static_assert(case_label_for<deferred::constant_<int>, I>);
  static_assert(case_label_for<I, I>);
  static_assert(case_label_for<int, int>);

  static_assert(case_label_for<std::string, S>);
  static_assert(case_label_for<char const*, S>);

  static_assert(!case_label_for<std::string, I>);
  static_assert(!case_label_for<char const*, I>);
  static_assert(!case_label_for<int, S>);
  static_assert(!case_label_for<not_a_condition, I>);
}

TEST_CASE("case_ rejects a label that cannot be compared to the condition", "[concepts]")
{
  using I = deferred::variable_<int>&;

  static_assert(usable_as_first_case<int, I>);
  static_assert(usable_as_first_case<deferred::constant_<long>, I>);
  static_assert(!usable_as_first_case<std::string, I>);
  static_assert(!usable_as_first_case<char const*, I>);

  static_assert(usable_as_later_case<int, I>);
  static_assert(!usable_as_later_case<std::string, I>);
}
