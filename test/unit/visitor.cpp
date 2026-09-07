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
  auto expression = deferred::while_(false).do_(1);
  std::vector<std::pair<node_kind, std::size_t>> visited;

  expression.visit([&](auto const& node, std::size_t nesting) {
    visited.emplace_back(node_kind_of_v<decltype(node)>, nesting);
  });

  using entry = std::pair<node_kind, std::size_t>;
  CHECK(visited
        == std::vector{entry{node_kind::while_, 0},
                       entry{node_kind::constant, 1},
                       entry{node_kind::constant, 1}});
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
