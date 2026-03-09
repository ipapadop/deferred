/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_WHILE_HPP
#define DEFERRED_WHILE_HPP

#include <type_traits>
#include <utility>

#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

/**
 * Deferred while loop that evaluates @p BodyExpression while
 * @p ConditionExpression evaluates to @c true.
 */
template<typename ConditionExpression, typename BodyExpression>
class while_expression
{
  [[no_unique_address]] std::tuple<ConditionExpression, BodyExpression> m_expressions;

public:
  using condition_expression_type = ConditionExpression;
  using body_expression_type      = BodyExpression;
  using subexpression_types       = std::tuple<ConditionExpression, BodyExpression>;

  template<typename Condition, typename Body>
  constexpr explicit while_expression(Condition&& condition, Body&& body) :
    m_expressions(std::forward<Condition>(condition), std::forward<Body>(body))
  { }

  constexpr void operator()() const
  {
    while (evaluate(std::get<0>(m_expressions)))
    {
      evaluate(std::get<1>(m_expressions));
    }
  }

  constexpr void operator()()
  {
    while (evaluate(std::get<0>(m_expressions)))
    {
      evaluate(std::get<1>(m_expressions));
    }
  }

  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    std::apply([&v, nesting](
                 auto const&... args) { (args.visit(std::forward<Visitor>(v), nesting + 1), ...); },
               m_expressions);
  }
};

/**
 * Creates a @c deferred while that call @p body while @p condition is @c true.
 */
template<typename ConditionExpression, typename BodyExpression>
constexpr auto while_(ConditionExpression&& condition, BodyExpression&& body)
{
  using condition_expression = make_deferred_t<ConditionExpression>;
  using body_expression      = make_deferred_t<BodyExpression>;
  return while_expression<condition_expression, body_expression>(
    std::forward<ConditionExpression>(condition),
    std::forward<BodyExpression>(body));
}

} // namespace deferred

#endif
