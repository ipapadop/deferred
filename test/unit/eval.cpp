// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <utility>

#include "deferred/deferred.hpp"

namespace {

// Whether eval() matches operator() in result type and exception specification, for a
// const and a mutable expression.
template<typename Expression>
constexpr bool eval_matches_call =
  std::is_same_v<decltype(std::declval<Expression&>().eval()),
                 decltype(std::declval<Expression&>()())>
  && std::is_same_v<decltype(std::declval<Expression const&>().eval()),
                    decltype(std::declval<Expression const&>()())>
  && noexcept(std::declval<Expression&>().eval()) == noexcept(std::declval<Expression&>()())
  && noexcept(std::declval<Expression const&>().eval())
       == noexcept(std::declval<Expression const&>()());

template<typename T>
concept HasEval = requires(T& t) { t.eval(); };

} // namespace

TEST_CASE("an operator expression has eval", "[eval]")
{
  auto v  = deferred::variable(2);
  auto ex = v * deferred::constant(3) + deferred::constant(4);

  static_assert(eval_matches_call<decltype(ex)>);
  static_assert(std::is_same_v<decltype(ex.eval()), int>);
  CHECK(ex.eval() == 10);
  CHECK(std::as_const(ex).eval() == 10);

  v = 5;
  CHECK(ex.eval() == 19);
}

TEST_CASE("eval is usable in a constant expression", "[eval]")
{
  constexpr auto ex = deferred::constant(2) + deferred::constant(3);

  static_assert(noexcept(ex.eval()));
  static_assert(ex.eval() == 5);
  static_assert((deferred::constant(true) && deferred::constant(false)).eval() == false);
  static_assert(deferred::if_(deferred::constant(true))
                  .then_(deferred::constant(1))
                  .else_(deferred::constant(2))
                  .eval()
                == 1);
}

TEST_CASE("an invoke expression has eval", "[eval]")
{
  auto calls = 0;
  auto ex    = deferred::invoke([&calls] { return ++calls; });

  static_assert(eval_matches_call<decltype(ex)>);
  CHECK(calls == 0);
  CHECK(ex.eval() == 1);
  CHECK(ex.eval() == 2);
}

TEST_CASE("assignment expressions have eval", "[eval]")
{
  auto i = deferred::variable(0);

  auto assignment = (i = deferred::constant(5));
  static_assert(eval_matches_call<decltype(assignment)>);
  static_assert(std::is_same_v<decltype(assignment.eval()), int&>);
  CHECK(&assignment.eval() == &i());
  CHECK(i() == 5);

  auto compound = (i += 2);
  static_assert(eval_matches_call<decltype(compound)>);
  compound.eval();
  CHECK(i() == 7);

  auto increment = ++i;
  static_assert(eval_matches_call<decltype(increment)>);
  increment.eval();
  CHECK(i() == 8);
}

TEST_CASE("a logical expression has eval", "[eval]")
{
  auto evaluated = 0;
  auto right     = deferred::invoke([&evaluated] {
    ++evaluated;
    return true;
  });
  auto ex        = deferred::constant(false) && right;

  static_assert(eval_matches_call<decltype(ex)>);
  static_assert(std::is_same_v<decltype(ex.eval()), bool>);
  CHECK(ex.eval() == false);
  CHECK(evaluated == 0);
}

TEST_CASE("a sequence has eval", "[eval]")
{
  auto i  = deferred::variable(0);
  auto j  = deferred::variable(0);
  auto ex = deferred::seq(++i, j += 10);

  static_assert(eval_matches_call<decltype(ex)>);
  ex.eval();
  CHECK(i() == 1);
  CHECK(j() == 10);
}

TEST_CASE("conditionals and switches have eval", "[eval]")
{
  auto i = deferred::variable(1);

  auto conditional =
    deferred::if_(i > 0).then_(deferred::constant(10)).else_(deferred::constant(20));
  static_assert(eval_matches_call<decltype(conditional)>);
  CHECK(conditional.eval() == 10);

  auto partial = deferred::if_(i > 5).then_(deferred::constant(10));
  static_assert(eval_matches_call<decltype(partial)>);
  CHECK(!partial.eval().has_value());

  auto selection =
    deferred::switch_(i).case_(1).then_(deferred::constant(100)).default_(deferred::constant(200));
  static_assert(eval_matches_call<decltype(selection)>);
  CHECK(selection.eval() == 100);

  i = 2;
  CHECK(conditional.eval() == 10);
  CHECK(selection.eval() == 200);
}

TEST_CASE("loops have eval", "[eval]")
{
  auto i     = deferred::variable(0);
  auto total = deferred::variable(0);

  auto counted = deferred::for_(i = deferred::constant(0), i < 4, ++i).do_(total += i);
  static_assert(eval_matches_call<decltype(counted)>);
  static_assert(std::is_void_v<decltype(counted.eval())>);
  counted.eval();
  CHECK(total() == 6);

  auto draining = deferred::while_(i > 0).do_(--i);
  static_assert(eval_matches_call<decltype(draining)>);
  draining.eval();
  CHECK(i() == 0);

  auto once = deferred::do_(++i).while_(i < 0);
  static_assert(eval_matches_call<decltype(once)>);
  once.eval();
  CHECK(i() == 1);
}

TEST_CASE("eval on a temporary expression evaluates it as a temporary", "[eval]")
{
  auto c = deferred::constant(1);
  auto v = deferred::variable(2);

  // As the call does: operator() and eval() agree for every value category.
  static_assert(std::is_same_v<decltype((v + c).eval()), decltype((v + c)())>);
  static_assert(std::is_same_v<decltype((++v).eval()), decltype((++v)())>);
  static_assert(noexcept((v + c).eval()) == noexcept((v + c)()));
  CHECK((v + c).eval() == 3);
}

TEST_CASE("eval adds no storage to an expression", "[eval]")
{
  auto ex = deferred::constant(1) + deferred::constant(2);
  static_assert(sizeof(ex) == 2 * sizeof(int));

  auto nested = (deferred::constant(1) + deferred::constant(2)) * deferred::constant(3);
  static_assert(sizeof(nested) == 3 * sizeof(int));
}

TEST_CASE("constants, variables and builders have no eval", "[eval]")
{
  static_assert(!HasEval<deferred::constant<int>>);
  static_assert(!HasEval<deferred::variable<int>>);
  static_assert(!HasEval<deferred::variable_ref<int>>);
  static_assert(!HasEval<deferred::variable_cref<int>>);

  auto i = deferred::variable(0);
  static_assert(!HasEval<decltype(deferred::if_(i > 0))>);
  static_assert(!HasEval<decltype(deferred::while_(i > 0))>);
}
