/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
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
#include "type_traits/is_constant_expression.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

template<typename ConditionExpression,
         typename ThenExpression,
         typename ElseExpression>
class if_then_else_expression;

namespace detail {

template<typename ConditionExpression,
         typename ThenExpression,
         typename ElseExpression>
struct is_deferred<
  if_then_else_expression<ConditionExpression, ThenExpression, ElseExpression>> :
  public std::true_type
{};

} // namespace detail

/**
 * Deferred conditional that evaluates @p ThenExpression if
 * @p ConditionExpression evaluates to @c true, otherwise it evaluates
 * @p ElseExpression.
 */
template<typename ConditionExpression,
         typename ThenExpression,
         typename ElseExpression>
class if_then_else_expression :
  private std::tuple<ConditionExpression, ThenExpression, ElseExpression>
{
private:
  using subexpression_types =
    std::tuple<ConditionExpression, ThenExpression, ElseExpression>;

public:
  using constant_expression =
    std::conjunction<is_constant_expression<ConditionExpression>,
                     is_constant_expression<ThenExpression>,
                     is_constant_expression<ElseExpression>>;
  using result_type =
    std::common_type_t<decltype(std::declval<ThenExpression>()()),
                       decltype(std::declval<ElseExpression>()())>;

  template<typename Condition, typename ThenEx, typename ElseEx>
  constexpr explicit if_then_else_expression(Condition&& condition,
                                             ThenEx&& then_,
                                             ElseEx&& else_) :
    std::tuple<ConditionExpression, ThenExpression, ElseExpression>(
      std::forward<Condition>(condition),
      std::forward<ThenEx>(then_),
      std::forward<ElseEx>(else_))
  {}

  constexpr result_type operator()() const
  {
    if (evaluate(std::get<0>(static_cast<subexpression_types const&>(*this))))
    {
      return evaluate(
        std::get<1>(static_cast<subexpression_types const&>(*this)));
    }
    else
    {
      return evaluate(
        std::get<2>(static_cast<subexpression_types const&>(*this)));
    }
  }

  constexpr result_type operator()()
  {
    if (evaluate(std::get<0>(static_cast<subexpression_types&>(*this))))
    {
      return evaluate(std::get<1>(static_cast<subexpression_types&>(*this)));
    }
    else
    {
      return evaluate(std::get<2>(static_cast<subexpression_types&>(*this)));
    }
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/**
 * Creates a @c deferred conditional that evaluates @p then_ if @p condition
 * evaluates to @c true, otherwise it evaluates @p else_.
 *
 * The result type of <tt>if_then_else_expression(...)()</tt> is the
 * @c std::common_type of the result types of @p then_ and @p else_.
 *
 * Example:
 * @code
 * auto v = variable<bool>();
 * auto ex = if_then_else(v, constant(42), constant(10));
 * v = true;
 * assert(v() == 42);
 * @endcode
 */
template<typename ConditionExpression,
         typename ThenExpression,
         typename ElseExpression>
constexpr auto if_then_else(ConditionExpression&& condition,
                            ThenExpression&& then_,
                            ElseExpression&& else_)
{
  using condition_expression = make_deferred_t<ConditionExpression>;
  using then_expression      = make_deferred_t<ThenExpression>;
  using else_expression      = make_deferred_t<ElseExpression>;
  return if_then_else_expression<condition_expression,
                                 then_expression,
                                 else_expression>(
    std::forward<ConditionExpression>(condition),
    std::forward<ThenExpression>(then_),
    std::forward<ElseExpression>(else_));
}

} // namespace deferred

#endif
