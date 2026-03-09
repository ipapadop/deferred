/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_CONDITIONAL_HPP
#define DEFERRED_CONDITIONAL_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

/**
 * Deferred conditional that evaluates @p ThenExpression if
 * @p ConditionExpression evaluates to @c true, otherwise it evaluates
 * @p ElseExpression.
 */
template<typename ConditionExpression, typename ThenExpression, typename ElseExpression>
class if_then_else_expression
{
  [[no_unique_address]] std::tuple<ConditionExpression, ThenExpression, ElseExpression>
    m_expressions;

public:
  using condition_expression_type = ConditionExpression;
  using then_expression_type      = ThenExpression;
  using else_expression_type      = ElseExpression;
  using result_type               = std::common_type_t<decltype(std::declval<ThenExpression>()()),
                                                       decltype(std::declval<ElseExpression>()())>;
  using subexpression_types       = std::tuple<ConditionExpression, ThenExpression, ElseExpression>;

  template<typename Condition, typename ThenEx, typename ElseEx>
  constexpr explicit if_then_else_expression(Condition&& condition,
                                             ThenEx&& then_,
                                             ElseEx&& else_) :
    m_expressions(std::forward<Condition>(condition),
                  std::forward<ThenEx>(then_),
                  std::forward<ElseEx>(else_))
  { }

  constexpr result_type operator()() const
  {
    if (evaluate(std::get<0>(m_expressions)))
    {
      return evaluate(std::get<1>(m_expressions));
    }
    else
    {
      return evaluate(std::get<2>(m_expressions));
    }
  }

  constexpr result_type operator()()
  {
    if (evaluate(std::get<0>(m_expressions)))
    {
      return evaluate(std::get<1>(m_expressions));
    }
    else
    {
      return evaluate(std::get<2>(m_expressions));
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
 * Creates a @c deferred conditional that evaluates @p then_ if @p condition
 * evaluates to @c true, otherwise it evaluates @p else_.
 *
 * The result type of <tt>if_then_else_expression(...)()</tt> is the
 * @c std::common_type of the result types of @p then_ and @p else_.
 */
template<typename ConditionExpression, typename ThenExpression, typename ElseExpression>
constexpr auto
if_then_else(ConditionExpression&& condition, ThenExpression&& then_, ElseExpression&& else_)
{
  using condition_expression = make_deferred_t<ConditionExpression>;
  using then_expression      = make_deferred_t<ThenExpression>;
  using else_expression      = make_deferred_t<ElseExpression>;
  return if_then_else_expression<condition_expression, then_expression, else_expression>(
    std::forward<ConditionExpression>(condition),
    std::forward<ThenExpression>(then_),
    std::forward<ElseExpression>(else_));
}

} // namespace deferred

#endif
