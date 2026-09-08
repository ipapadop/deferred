// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <type_traits>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

namespace {

enum class node_kind
{
  conditional,
  branch,
  constant,
  variable,
  expression,
  while_,
  do_while_,
  assign_,
  seq_,
  for_,
  switch_,
  default_,
  case_
};

template<typename T>
struct node_kind_of;

template<typename Else, typename... Branches>
struct node_kind_of<deferred::conditional_expression<Else, Branches...>> :
  std::integral_constant<node_kind, node_kind::conditional>
{ };

template<typename Condition, typename Then>
struct node_kind_of<deferred::conditional_branch<Condition, Then>> :
  std::integral_constant<node_kind, node_kind::branch>
{ };

template<typename T>
struct node_kind_of<deferred::constant_<T>> : std::integral_constant<node_kind, node_kind::constant>
{ };

template<typename T>
struct node_kind_of<deferred::variable_<T>> : std::integral_constant<node_kind, node_kind::variable>
{ };

template<typename Operator, typename... Expressions>
struct node_kind_of<deferred::expression_<Operator, Expressions...>> :
  std::integral_constant<node_kind, node_kind::expression>
{ };

template<typename Condition, typename Body>
struct node_kind_of<deferred::while_expression<Condition, Body>> :
  std::integral_constant<node_kind, node_kind::while_>
{ };

template<typename Operation, typename Target, typename Value>
struct node_kind_of<deferred::assign_expression<Operation, Target, Value>> :
  std::integral_constant<node_kind, node_kind::assign_>
{ };

template<typename Init, typename Condition, typename Step, typename Body>
struct node_kind_of<deferred::for_expression<Init, Condition, Step, Body>> :
  std::integral_constant<node_kind, node_kind::for_>
{ };

template<typename... Expressions>
struct node_kind_of<deferred::seq_expression<Expressions...>> :
  std::integral_constant<node_kind, node_kind::seq_>
{ };

template<typename Body, typename Condition>
struct node_kind_of<deferred::do_while_expression<Body, Condition>> :
  std::integral_constant<node_kind, node_kind::do_while_>
{ };

template<typename Condition, typename Default, typename... Cases>
struct node_kind_of<deferred::switch_expression<Condition, Default, Cases...>> :
  std::integral_constant<node_kind, node_kind::switch_>
{ };

template<typename Expression>
struct node_kind_of<deferred::default_expression<Expression>> :
  std::integral_constant<node_kind, node_kind::default_>
{ };

template<typename Label, typename Body>
struct node_kind_of<deferred::case_expression<Label, Body>> :
  std::integral_constant<node_kind, node_kind::case_>
{ };

template<typename T>
inline constexpr node_kind node_kind_of_v = node_kind_of<std::remove_cvref_t<T>>::value;

struct lvalue_visitor
{
  std::size_t* visited_nodes;

  template<typename T>
  void operator()(T const&, std::size_t) &
  {
    ++*visited_nodes;
  }

  template<typename T>
  void operator()(T const&, std::size_t) && = delete;
};

struct no_throw_visitor
{
  template<typename T>
  constexpr void operator()(T const&, std::size_t) const noexcept
  { }
};

struct throwing_visitor
{
  template<typename T>
  void operator()(T const&, std::size_t) const
  { }
};

} // namespace

TEST_CASE("constant visitor treats the stored value as data", "[visitor]")
{
  auto constant = deferred::constant(42);
  std::size_t visited_nodes{};

  constant.visit([&](auto const&, std::size_t) { ++visited_nodes; });

  CHECK(visited_nodes == 1);
}

TEST_CASE("variable visitor treats the stored value as data", "[visitor]")
{
  auto variable = deferred::variable(42);
  std::size_t visited_nodes{};

  variable.visit([&](auto const&, std::size_t) { ++visited_nodes; });

  CHECK(visited_nodes == 1);
}

TEST_CASE("temporary visitor is reused as an lvalue", "[visitor]")
{
  auto variable   = deferred::variable(42);
  auto expression = variable + 1;
  std::size_t visited_nodes{};

  expression.visit(lvalue_visitor{&visited_nodes});

  CHECK(visited_nodes == 3);
}

TEST_CASE("visitor propagates exception specifications", "[visitor]")
{
  auto variable   = deferred::variable(42);
  auto expression = variable + 1;

  static_assert(noexcept(expression.visit(no_throw_visitor{})));
  static_assert(!noexcept(expression.visit(throwing_visitor{})));
}

TEST_CASE("conditional visitor includes branch nodes in preorder", "[visitor]")
{
  auto expression = deferred::if_(false).then_(1).else_if_(true).then_(2).else_(3);
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::conditional, 0},
                       entry{node_kind::branch, 1},
                       entry{node_kind::constant, 2},
                       entry{node_kind::constant, 2},
                       entry{node_kind::branch, 1},
                       entry{node_kind::constant, 2},
                       entry{node_kind::constant, 2},
                       entry{node_kind::constant, 1}});
}

TEST_CASE("expression visitor traverses operands but not the operator", "[visitor]")
{
  auto variable   = deferred::variable(42);
  auto expression = variable + 1;
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::expression, 0},
                       entry{node_kind::variable, 1},
                       entry{node_kind::constant, 1}});
}

TEST_CASE("while visitor traverses condition before body", "[visitor]")
{
  // Each child is a different node kind, so the order is actually pinned.
  auto expression = deferred::while_(false).do_([] { });
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::while_, 0},
                       entry{node_kind::constant, 1},
                       entry{node_kind::expression, 1}});
}

TEST_CASE("do-while visitor traverses body before condition", "[visitor]")
{
  // Each child is a different node kind, so the order is actually pinned.
  auto expression = deferred::do_([] { }).while_(false);
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::do_while_, 0},
                       entry{node_kind::expression, 1},
                       entry{node_kind::constant, 1}});
}

TEST_CASE("for visitor reaches the loop it is composed of", "[visitor]")
{
  // for_ is a node of its own; the composition it holds sits underneath it.
  auto i          = deferred::variable(0);
  auto expression = deferred::for_(i, false, [] { }).do_(1);
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::for_, 0},
                       entry{node_kind::seq_, 1},
                       entry{node_kind::variable, 2},
                       entry{node_kind::while_, 2},
                       entry{node_kind::constant, 3},
                       entry{node_kind::seq_, 3},
                       entry{node_kind::constant, 4},
                       entry{node_kind::expression, 4}});
}

TEST_CASE("assignment visitor traverses target before value", "[visitor]")
{
  auto v          = deferred::variable<int>();
  auto expression = (v = deferred::constant(1));
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::assign_, 0},
                       entry{node_kind::variable, 1},
                       entry{node_kind::constant, 1}});
}

TEST_CASE("seq visitor traverses its expressions in order", "[visitor]")
{
  auto v          = deferred::variable<int>();
  auto expression = deferred::seq(v, deferred::constant(1), [] { });
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::seq_, 0},
                       entry{node_kind::variable, 1},
                       entry{node_kind::constant, 1},
                       entry{node_kind::expression, 1}});
}

TEST_CASE("switch visitor skips the default of an unfinalized switch", "[visitor]")
{
  auto expression = deferred::switch_(1).case_(1).then_(10);
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::switch_, 0},
                       entry{node_kind::constant, 1},
                       entry{node_kind::case_, 1},
                       entry{node_kind::constant, 2},
                       entry{node_kind::constant, 2}});
}

TEST_CASE("conditional visitor skips the else of an unfinalized chain", "[visitor]")
{
  auto expression = deferred::if_(false).then_(1).else_if_(true).then_(2);
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::conditional, 0},
                       entry{node_kind::branch, 1},
                       entry{node_kind::constant, 2},
                       entry{node_kind::constant, 2},
                       entry{node_kind::branch, 1},
                       entry{node_kind::constant, 2},
                       entry{node_kind::constant, 2}});
}

TEST_CASE("switch visitor includes default and case wrappers", "[visitor]")
{
  auto expression = deferred::switch_(1).case_(1).then_(10).case_(2).then_(20).default_(0);
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::switch_, 0},
                       entry{node_kind::constant, 1},
                       entry{node_kind::default_, 1},
                       entry{node_kind::constant, 2},
                       entry{node_kind::case_, 1},
                       entry{node_kind::constant, 2},
                       entry{node_kind::constant, 2},
                       entry{node_kind::case_, 1},
                       entry{node_kind::constant, 2},
                       entry{node_kind::constant, 2}});
}
