// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "deferred/deferred.hpp"

TEST_CASE("make_deferred_t stores a deferred operand by value", "[expression]")
{
  using C  = deferred::constant<int>;
  using Ex = deferred::expression_<std::plus<>, C, C>;

  static_assert(std::is_same_v<deferred::make_deferred_t<C>, C>);

  // A named operand is copied, so the expression does not refer to it.
  static_assert(std::is_same_v<deferred::make_deferred_t<C&>, C>);
  static_assert(std::is_same_v<deferred::make_deferred_t<C const&>, C>);
  static_assert(std::is_same_v<deferred::make_deferred_t<Ex&>, Ex>);
  static_assert(std::is_same_v<deferred::make_deferred_t<Ex const&>, Ex>);
  static_assert(std::is_same_v<deferred::make_deferred_t<Ex&&>, Ex>);
}

TEST_CASE("make_deferred_t refers to a named variable through a variable_ref", "[expression]")
{
  using V = deferred::variable<int>;

  static_assert(std::is_same_v<deferred::make_deferred_t<V&>, deferred::variable_ref<int>>);
  static_assert(std::is_same_v<deferred::make_deferred_t<V const&>, deferred::variable_cref<int>>);

  // A temporary variable is rejected with a static_assert, which a test cannot pin.
}

TEST_CASE("make_deferred_t never yields a reference", "[expression]")
{
  auto callable = [] {
    return 1;
  };

  static_assert(!std::is_reference_v<deferred::make_deferred_t<int&>>);
  static_assert(!std::is_reference_v<deferred::make_deferred_t<decltype(callable)&>>);
  static_assert(!std::is_reference_v<deferred::make_deferred_t<deferred::constant<int>&>>);
  static_assert(!std::is_reference_v<deferred::make_deferred_t<deferred::variable<int>&>>);
}

TEST_CASE("make_deferred_t wraps a callable in an expression", "[expression]")
{
  auto callable = [] {
    return 1;
  };

  static_assert(std::is_same_v<deferred::make_deferred_t<decltype(callable)>,
                               deferred::expression_<decltype(callable)>>);
}

TEST_CASE("make_deferred_t wraps a value in a constant", "[expression]")
{
  static_assert(std::is_same_v<deferred::make_deferred_t<int>, deferred::constant<int>>);
  static_assert(std::is_same_v<deferred::make_deferred_t<int&>, deferred::constant<int>>);
}

TEST_CASE("expression exposes its operator and subexpressions", "[expression]")
{
  auto ex = deferred::constant(1) + deferred::constant(2);

  static_assert(std::is_same_v<decltype(ex)::operator_type, std::plus<>>);
  // Only the visited children are subexpressions; the operator is not one of them.
  static_assert(std::is_same_v<decltype(ex)::subexpression_types,
                               std::tuple<deferred::constant<int>, deferred::constant<int>>>);

  CHECK(std::get<0>(ex.subexpressions())() == 1);
  CHECK(ex() == 3);
}

TEST_CASE("expression evaluates its subexpressions on every evaluation", "[expression]")
{
  auto v  = deferred::variable(1);
  auto ex = v + deferred::constant(10);

  CHECK(ex() == 11);
  v = 5;
  CHECK(ex() == 15);
}

TEST_CASE("an expression with a stateful operator is not a constant expression", "[expression]")
{
  int state     = 0;
  auto stateful = [&state] {
    return state;
  };
  auto stateless = [] {
    return 1;
  };

  // The operator is no longer listed in subexpression_types, but it still counts.
  static_assert(deferred::is_constant_expression_v<deferred::expression_<decltype(stateless)>>);
  static_assert(!deferred::is_constant_expression_v<deferred::expression_<decltype(stateful)>>);
}

namespace {

template<template<typename...> class Node, typename... Arguments>
concept nameable = requires { typename Node<Arguments...>; };

} // namespace

TEST_CASE("an expression requires deferred subexpressions", "[expression]")
{
  // A template argument that is not a deferred expression is rejected at the class,
  // not when the expression is evaluated.
  static_assert(nameable<deferred::expression_, std::plus<>, deferred::constant<int>>);
  static_assert(!nameable<deferred::expression_, std::plus<>, int>);
  static_assert(!nameable<deferred::conditional_expression, deferred::detail::no_else, int>);
}

namespace {

/// @brief Whether subexpressions() of @p T returns references matching its
/// @c subexpression_types , element for element.
template<typename T>
consteval bool subexpressions_match()
{
  using children   = typename T::subexpression_types;
  using references = decltype(std::declval<T const&>().subexpressions());
  if (std::tuple_size_v<children> != std::tuple_size_v<references>)
  {
    return false;
  }
  return []<std::size_t... I>(std::index_sequence<I...>) {
    return (std::is_same_v<std::remove_cvref_t<std::tuple_element_t<I, children>>,
                           std::remove_cvref_t<std::tuple_element_t<I, references>>>
            && ...)
           && (std::is_lvalue_reference_v<std::tuple_element_t<I, references>> && ...);
  }(std::make_index_sequence<std::tuple_size_v<children>>{});
}

} // namespace

TEST_CASE("every node exposes its subexpressions", "[expression]")
{
  auto i = deferred::variable(1);
  auto c = deferred::constant(2);

  auto sum      = i + c;
  auto assigned = (i = c);
  auto both     = (i > 0 && c > 0);
  auto sequence = deferred::seq(i, c);
  auto loop     = deferred::while_(i < 0).do_(++i);
  auto do_loop  = deferred::do_(++i).while_(i < 0);
  auto for_loop = deferred::for_(i = deferred::constant(0), i < 0, ++i).do_(c);
  auto open_if  = deferred::if_(i > 0).then_(c);
  auto full_if  = deferred::if_(i > 0).then_(c).else_(i);
  auto open_sw  = deferred::switch_(i).case_(1).then_(c);
  auto full_sw  = deferred::switch_(i).case_(1).then_(c).default_(i);

  static_assert(subexpressions_match<decltype(i)>());
  static_assert(subexpressions_match<decltype(c)>());
  static_assert(subexpressions_match<decltype(sum)>());
  static_assert(subexpressions_match<decltype(assigned)>());
  static_assert(subexpressions_match<decltype(both)>());
  static_assert(subexpressions_match<decltype(sequence)>());
  static_assert(subexpressions_match<decltype(loop)>());
  static_assert(subexpressions_match<decltype(do_loop)>());
  static_assert(subexpressions_match<decltype(for_loop)>());
  static_assert(subexpressions_match<decltype(open_if)>());
  static_assert(subexpressions_match<decltype(full_if)>());
  static_assert(subexpressions_match<decltype(open_sw)>());
  static_assert(subexpressions_match<decltype(full_sw)>());

  // A leaf has none; a variable is handed back as the variable_ref that refers to it.
  static_assert(std::tuple_size_v<decltype(i.subexpressions())> == 0);
  CHECK(&std::get<0>(sum.subexpressions())() == &i());
  CHECK(&std::get<0>(assigned.subexpressions())() == &i());
  CHECK(std::get<1>(sum.subexpressions())() == 2);

  // The children of a branch and of a case are reachable the same way.
  auto const& branch = std::get<0>(full_if.subexpressions());
  CHECK(std::get<1>(branch.subexpressions())() == 2);
  auto const& first_case = std::get<2>(full_sw.subexpressions());
  CHECK(std::get<0>(first_case.subexpressions())() == 1);
}

TEST_CASE("a temporary expression yields a value where a stored one yields a reference",
          "[expression-temporary]")
{
  auto table =
    deferred::constant(std::vector<std::string>{std::string(40, 'a'), std::string(40, 'b')});
  auto index = deferred::variable(std::size_t{1});

  // A stored expression owns a copy of the table and may refer into it.
  auto stored = deferred::subscript(table, index);
  static_assert(std::is_same_v<decltype(stored()), std::string const&>);
  static_assert(std::is_same_v<decltype(stored.eval()), std::string const&>);
  CHECK(stored() == std::string(40, 'b'));

  // A temporary one is destroyed at the end of the statement, with its copy.
  static_assert(std::is_same_v<decltype(deferred::subscript(table, index)()), std::string>);
  static_assert(std::is_same_v<decltype(deferred::subscript(table, index).eval()), std::string>);
  static_assert(std::is_same_v<decltype(std::move(stored)()), std::string>);

  // The reference is to a value that lives as long as it does; ASan checks that.
  auto const& element = deferred::subscript(table, index)();
  CHECK(element == std::string(40, 'b'));

  auto const& named = deferred::subscript(table, index).eval();
  CHECK(named == std::string(40, 'b'));
}

TEST_CASE("a temporary expression still yields a reference into a variable",
          "[expression-temporary]")
{
  auto values = deferred::variable(std::vector<int>{1, 2, 3});
  auto index  = deferred::variable(std::size_t{1});
  auto count  = deferred::variable(0);

  // The variable outlives the expression, so the reference is valid.
  static_assert(std::is_same_v<decltype(deferred::subscript(values, index)()), int&>);
  static_assert(std::is_same_v<decltype((++count)()), int&>);
  static_assert(std::is_same_v<decltype((count += deferred::constant(1))()), int&>);

  deferred::subscript(values, index)() = 20;
  CHECK(values()[1] == 20);

  int& incremented = (++count)();
  CHECK(&incremented == &count());
  CHECK(count() == 1);
}

TEST_CASE("a temporary expression over temporaries yields a value", "[expression-temporary]")
{
  auto index = deferred::variable(std::size_t{0});

  static_assert(
    std::is_same_v<decltype(deferred::subscript(deferred::constant(std::vector<int>{4, 5}), index)()),
                   int>);
  CHECK(deferred::subscript(deferred::constant(std::vector<int>{4, 5}), index)() == 4);

  // Nested temporaries are evaluated as temporaries all the way down.
  auto text = deferred::constant(std::optional<std::string>(std::string(40, 'x')));
  static_assert(std::is_same_v<decltype((*text)()), std::string>);
  auto const& value = (*text)();
  CHECK(value == std::string(40, 'x'));
}

namespace {

/// @brief A node that declares state of its own.
struct stateful_leaf
{
  using subexpression_types = std::tuple<>;

  static constexpr bool mutable_state = true;

  int operator()() const noexcept
  {
    return 0;
  }
};

template<typename T>
inline constexpr bool owns_state_v = deferred::detail::owns_state<std::remove_cvref_t<T>>();

} // namespace

TEST_CASE("owns_state finds the nodes whose copy would be a second state", "[owns-state]")
{
  auto c = deferred::constant(1);
  auto v = deferred::variable(1);

  // Constants, references to variables and expressions over them do not.
  static_assert(!owns_state_v<deferred::constant<int>>);
  static_assert(!owns_state_v<deferred::variable_ref<int>>);
  static_assert(!owns_state_v<deferred::variable_cref<int>>);
  static_assert(!owns_state_v<decltype(v + c)>);
  static_assert(!owns_state_v<decltype(++v)>);
  static_assert(!owns_state_v<decltype(v = c)>);
  static_assert(!owns_state_v<decltype(v += c)>);
  static_assert(!owns_state_v<decltype(deferred::if_(v > c).then_(c).else_(v))>);
  static_assert(!owns_state_v<decltype(deferred::for_(v = c, v < c, ++v).do_(v += c))>);

  // A variable does, as does a node that says so.
  static_assert(owns_state_v<deferred::variable<int>>);
  static_assert(owns_state_v<stateful_leaf>);
}

TEST_CASE("owns_state follows callables and stored nodes", "[owns-state]")
{
  auto c        = deferred::constant(1);
  auto counting = [n = 0]() mutable {
    return ++n;
  };
  int external   = 0;
  auto capturing = [&external] {
    return ++external;
  };

  // A callable that can only be called as non-const changes state it holds.
  static_assert(owns_state_v<decltype(deferred::invoke(counting))>);
  // One that captures by reference holds none, and neither does a plain function object.
  static_assert(!owns_state_v<decltype(deferred::invoke(capturing))>);
  static_assert(!owns_state_v<decltype(deferred::invoke(std::plus<>{}, c, c))>);

  // An expression that stores such a node owns its state too.
  static_assert(owns_state_v<decltype(deferred::invoke(counting) + c)>);
  static_assert(owns_state_v<decltype(deferred::if_(c).then_(deferred::invoke(counting)))>);
  static_assert(owns_state_v<decltype(deferred::seq(c, stateful_leaf{}))>);

  // A std::reference_wrapper refers to the node, so the expression owns nothing.
  auto next = deferred::invoke(counting);
  static_assert(!owns_state_v<decltype(std::ref(next) + c)>);
  static_assert(
    !owns_state_v<decltype(deferred::make_deferred_t<decltype(std::ref(next))>{std::ref(next)})>);
}

namespace {

/// @brief Records the address of every node it is called with.
struct address_visitor
{
  std::vector<void const*> seen;

  template<typename Node>
  void operator()(Node const& node, std::size_t)
  {
    seen.push_back(&node);
  }
};

/// @brief Accepts a variable only.
struct int_variable_visitor
{
  void operator()(deferred::variable<int> const&, std::size_t) const noexcept
  { }
};

} // namespace

TEST_CASE("a visitor is called with the node a std::ref operand refers to", "[expression-visit]")
{
  auto v    = deferred::variable(1);
  auto next = deferred::invoke([count = 0]() mutable { return ++count; });

  auto sum = std::ref(next) + std::cref(v);

  address_visitor visitor;
  sum.visit(visitor);

  // The expression itself, then the two nodes referred to; not the reference_wrappers.
  REQUIRE(visitor.seen.size() == 3);
  CHECK(visitor.seen[0] == &sum);
  CHECK(visitor.seen[1] == &next);
  CHECK(visitor.seen[2] == &v);
}

TEST_CASE("visiting through std::ref is checked against the node referred to", "[expression-visit]")
{
  auto v = deferred::variable(1);
  auto referred =
    deferred::make_deferred_t<std::reference_wrapper<deferred::variable<int>>>(std::ref(v));

  static_assert(deferred::expression_visitor<int_variable_visitor, decltype(referred)>);
  static_assert(noexcept(referred.visit(int_variable_visitor{})));

  address_visitor throwing;
  static_assert(!noexcept(referred.visit(throwing)));

  // A reference_wrapper to something that is not a node is an ordinary callable.
  int i         = 0;
  auto function = [&i] {
    return ++i;
  };
  auto wrapped = deferred::invoke(std::ref(function));
  address_visitor visitor;
  wrapped.visit(visitor);
  REQUIRE(visitor.seen.size() == 1);
  CHECK(visitor.seen[0] == &wrapped);
}

namespace {

/// @brief An empty callable whose operator() is not const.
struct non_const_generator
{
  int operator()() noexcept
  {
    return 4;
  }
};

} // namespace

TEST_CASE("an empty callable owns no state whatever its operator() is qualified with",
          "[owns-state]")
{
  auto generator = deferred::invoke(non_const_generator{});
  static_assert(!owns_state_v<decltype(generator)>);

  // An expression that stores it can still be asked, and can be named as an operand.
  int offset   = 1;
  auto shifted = deferred::invoke([offset](int x) { return x + offset; }, generator);
  static_assert(!owns_state_v<decltype(shifted)>);

  auto sum = shifted + deferred::constant(1);
  CHECK(sum() == 6);
}

TEST_CASE("invoke stores a node as any other operand", "[expression]")
{
  auto v    = deferred::variable(1);
  auto c    = deferred::constant(2);
  auto next = deferred::invoke([count = 0]() mutable { return ++count; });

  // A variable is referred to, a constant is copied, std::ref refers to any node.
  static_assert(std::is_same_v<decltype(deferred::invoke(v)), deferred::variable_ref<int>>);
  static_assert(std::is_same_v<decltype(deferred::invoke(c)), deferred::constant<int>>);
  static_assert(
    std::is_same_v<decltype(deferred::invoke(std::ref(next))), deferred::node_ref<decltype(next)>>);

  auto shared = deferred::invoke(std::ref(next));
  CHECK(shared() == 1);
  CHECK(next() == 2);

  address_visitor visitor;
  shared.visit(visitor);
  REQUIRE(visitor.seen.size() == 1);
  CHECK(visitor.seen[0] == &next);
}
