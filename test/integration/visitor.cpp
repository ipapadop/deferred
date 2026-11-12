// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("visitor traverses an expression composed through the public API", "[visitor]")
{
  auto variable   = deferred::variable(1);
  auto expression = deferred::if_(variable > 0).then_(variable + 1).else_(0);
  std::vector<std::size_t> nesting_levels;

  expression.visit([&](auto const&, std::size_t nesting) { nesting_levels.push_back(nesting); }, 7);

  CHECK(nesting_levels == std::vector<std::size_t>{7, 8, 9, 10, 10, 9, 10, 10, 8});
}

TEST_CASE("type_name is available from the umbrella header", "[type-name]")
{
  // This file includes only deferred/deferred.hpp.
  auto ex = deferred::constant(1) + deferred::constant(2);

  CHECK(deferred::type_name<int>() == "int");
  CHECK(deferred::type_name(ex).find("expression_") != std::string::npos);
}

namespace {

struct node_counter
{
  std::size_t* nodes;

  template<typename Node>
  void operator()(Node const&, std::size_t) const
  {
    ++*nodes;
  }
};

// Counts the nodes of any expression a node_counter can traverse.
template<typename Expression>
  requires deferred::expression_visitor<node_counter, Expression>
std::size_t count_nodes(Expression const& expression)
{
  std::size_t nodes = 0;
  expression.visit(node_counter{&nodes});
  return nodes;
}

// Doubles an expression yielding an int.
template<deferred::expression_yielding<int> Expression>
auto twice(Expression&& expression)
{
  return std::forward<Expression>(expression) * deferred::constant(2);
}

template<typename T>
concept can_be_doubled = requires(T t) { twice(t); };

} // namespace

TEST_CASE("the concepts constrain user templates", "[concepts-integration]")
{
  auto i = deferred::variable(3);

  auto doubled = twice(i + deferred::constant(1));
  CHECK(doubled() == 8);
  CHECK(count_nodes(doubled) == 5);

  static_assert(can_be_doubled<decltype(i)&>);
  static_assert(!can_be_doubled<deferred::constant<std::string>>);
  static_assert(!can_be_doubled<int>);

  static_assert(deferred::constant_expression<decltype(twice(deferred::constant(1)))>);
  static_assert(!deferred::constant_expression<decltype(doubled)>);
  static_assert(deferred::lvalue_expression<decltype(i)&>);
  static_assert(deferred::condition<decltype(doubled > 4)>);
  static_assert(deferred::case_label_for<int, decltype(doubled)>);
}
