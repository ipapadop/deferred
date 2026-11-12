// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>

#include "deferred/deferred.hpp"

namespace {

struct any_node_visitor
{
  template<typename Node>
  constexpr void operator()(Node const&, std::size_t) const noexcept
  { }
};

struct mutable_visitor
{
  template<typename Node>
  constexpr void operator()(Node const&, std::size_t) noexcept
  { }
};

// Accepts constants only, so it can traverse a tree made of nothing else.
struct constant_only_visitor
{
  template<typename T>
  constexpr void operator()(deferred::constant_<T> const&, std::size_t) const noexcept
  { }
};

struct wrong_arity_visitor
{
  template<typename Node>
  constexpr void operator()(Node const&) const noexcept
  { }
};

template<typename Visitor, typename Node>
concept can_visit = requires(Node const& node, Visitor visitor) { node.visit(visitor); };

} // namespace

TEST_CASE("expression_visitor is satisfied by a visitor taking every node", "[expression-visitor]")
{
  using deferred::expression_visitor;

  auto v  = deferred::variable(1);
  auto ex = deferred::if_(v > 0).then_(v + 1).else_(deferred::seq(++v, v * 2));

  static_assert(expression_visitor<any_node_visitor, decltype(ex)>);
  static_assert(expression_visitor<any_node_visitor&, decltype(ex) const&>);
  static_assert(expression_visitor<any_node_visitor const&, decltype(ex)>);
  static_assert(expression_visitor<mutable_visitor, decltype(ex)>);
  static_assert(!expression_visitor<mutable_visitor const&, decltype(ex)>);

  auto lambda = [](auto const&, std::size_t) {
  };
  static_assert(expression_visitor<decltype(lambda), decltype(ex)>);

  static_assert(!expression_visitor<wrong_arity_visitor, decltype(ex)>);
  static_assert(!expression_visitor<int, decltype(ex)>);
}

TEST_CASE("expression_visitor checks the nodes below the one visited", "[expression-visitor]")
{
  using deferred::expression_visitor;

  using C = deferred::constant_<int>;
  static_assert(expression_visitor<constant_only_visitor, C>);

  // The root is not a constant.
  auto sum = deferred::constant(1) + deferred::constant(2);
  static_assert(!expression_visitor<constant_only_visitor, decltype(sum)>);

  // The root is accepted by this visitor, but its children are not.
  auto root_only = [](decltype(sum) const&, std::size_t) {
  };
  static_assert(!expression_visitor<decltype(root_only), decltype(sum)>);
}

TEST_CASE("expression_visitor requires a deferred expression to visit", "[expression-visitor]")
{
  static_assert(!deferred::expression_visitor<any_node_visitor, int>);

  auto v = deferred::variable(0);
  static_assert(!deferred::expression_visitor<any_node_visitor, decltype(deferred::if_(v > 0))>);
}

TEST_CASE("visit rejects a visitor that cannot take every node", "[expression-visitor]")
{
  auto i    = deferred::variable(0);
  auto loop = deferred::for_(i = deferred::constant(0), i < 3, ++i).do_(i += 1);
  auto pick = deferred::switch_(i).case_(1).then_(i + 1).default_(i);

  static_assert(can_visit<any_node_visitor, decltype(loop)>);
  static_assert(can_visit<any_node_visitor, decltype(pick)>);
  static_assert(can_visit<any_node_visitor, decltype(i)>);

  static_assert(!can_visit<wrong_arity_visitor, decltype(loop)>);
  static_assert(!can_visit<wrong_arity_visitor, decltype(pick)>);
  static_assert(!can_visit<constant_only_visitor, decltype(loop)>);
  static_assert(!can_visit<constant_only_visitor, decltype(i)>);
  static_assert(can_visit<constant_only_visitor, deferred::constant_<int>>);
}
