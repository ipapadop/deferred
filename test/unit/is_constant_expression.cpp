// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <tuple>

#include "deferred/constant.hpp"
#include "deferred/operators.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"
#include "deferred/variable.hpp"

namespace {

struct stateless_node
{
  using subexpression_types = std::tuple<>;
};

struct mutable_node
{
  using subexpression_types           = std::tuple<>;
  static constexpr bool mutable_state = true;
};

struct node_with_mutable_child
{
  using subexpression_types = std::tuple<mutable_node>;
};

struct node_declaring_constant_state
{
  using subexpression_types           = std::tuple<>;
  static constexpr bool mutable_state = false;
};

} // namespace

TEST_CASE("leaves without mutable state are constant expressions", "[is-constant-expression]")
{
  STATIC_CHECK(deferred::is_constant_expression_v<deferred::constant_<int>>);
  STATIC_CHECK(deferred::is_constant_expression_v<stateless_node>);
  STATIC_CHECK(deferred::is_constant_expression_v<node_declaring_constant_state>);
}

TEST_CASE("variables are not constant expressions", "[is-constant-expression]")
{
  STATIC_CHECK(!deferred::is_constant_expression_v<deferred::variable_<int>>);
}

TEST_CASE("HasMutableState reads the opt-out", "[is-constant-expression]")
{
  STATIC_CHECK(deferred::HasMutableState<deferred::variable_<int>>);
  STATIC_CHECK(deferred::HasMutableState<mutable_node>);
  STATIC_CHECK(!deferred::HasMutableState<node_declaring_constant_state>);
  STATIC_CHECK(!deferred::HasMutableState<stateless_node>);
  STATIC_CHECK(!deferred::HasMutableState<deferred::constant_<int>>);
}

TEST_CASE("mutable state is opted out per node, not per subtree", "[is-constant-expression]")
{
  STATIC_CHECK(!deferred::is_constant_expression_v<mutable_node>);
  STATIC_CHECK(!deferred::is_constant_expression_v<node_with_mutable_child>);
}

TEST_CASE("is_constant_expression decays its argument", "[is-constant-expression]")
{
  STATIC_CHECK(!deferred::is_constant_expression_v<deferred::variable_<int>&>);
  STATIC_CHECK(!deferred::is_constant_expression_v<deferred::variable_<int> const&>);
  STATIC_CHECK(deferred::is_constant_expression_v<deferred::constant_<int> const&>);
}

TEST_CASE("an expression is constant when every part of it is", "[is-constant-expression]")
{
  auto c = deferred::constant(1);
  auto v = deferred::variable<int>();

  STATIC_CHECK(deferred::is_constant_expression_v<decltype(c + 1)>);
  STATIC_CHECK(!deferred::is_constant_expression_v<decltype(v + 1)>);
}

TEST_CASE("a variable reports an empty subexpression list", "[is-constant-expression]")
{
  // The opt-out carries the "not constant" meaning, so subexpression_types can say
  // what it says everywhere else: this node has no children.
  STATIC_CHECK(std::is_same_v<deferred::variable_<int>::subexpression_types, std::tuple<>>);
  STATIC_CHECK(std::is_same_v<deferred::constant_<int>::subexpression_types, std::tuple<>>);
}
