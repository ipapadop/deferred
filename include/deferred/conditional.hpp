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

#include "constant.hpp"
#include "expression.hpp"
#include "type_traits/is_constant_expression.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

/**
 * Deferred conditional that evaluates @p ThenExpression if @p IfExpression
 * evaluates to @c true, otherwise it evaluates @p ElseExpression.
 */
template<typename IfExpression, typename ThenExpression, typename ElseExpression>
class if_then_else_ :
  private std::tuple<IfExpression, ThenExpression, ElseExpression>
{
public:
  using is_constant_expression =
    std::conjunction<is_constant_expression<IfExpression>,
                     is_constant_expression<ThenExpression>,
                     is_constant_expression<ElseExpression>>;

  using subexpression_types =
    std::tuple<IfExpression, ThenExpression, ElseExpression>;

  template<typename IfEx, typename ThenEx, typename ElseEx>
  constexpr explicit if_then_else_(IfEx&& if_, ThenEx&& then_, ElseEx&& else_) :
    std::tuple<IfExpression, ThenExpression, ElseExpression>(
      std::forward<IfEx>(if_),
      std::forward<ThenEx>(then_),
      std::forward<ElseEx>(else_))
  {}

  constexpr decltype(auto) operator()() const
  {
    if (std::get<0>(static_cast<subexpression_types const&>(*this))())
    {
      return std::get<1>(static_cast<subexpression_types const&>(*this))();
    }
    else
    {
      return std::get<2>(static_cast<subexpression_types const&>(*this))();
    }
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

namespace detail {

// Transforms T into an expression_ if it is not a deferred data type for use
// with if_then_else_.
template<typename T>
using make_conditional_expression_t = std::conditional_t<
  is_deferred_v<T>,
  T,
  std::conditional_t<std::is_invocable_v<T>, make_expression_t<T>, constant_<T>>>;

} // namespace detail

/**
 * Creates a new deferred conditional that evaluates @p then_ if @p if_
 * evaluates to @c true, otherwise it evaluates @p else_.
 */
template<typename IfExpression, typename ThenExpression, typename ElseExpression>
constexpr auto
if_then_else(IfExpression&& if_, ThenExpression&& then_, ElseExpression&& else_)
{
  using if_expression   = detail::make_conditional_expression_t<IfExpression>;
  using then_expression = detail::make_conditional_expression_t<ThenExpression>;
  using else_expression = detail::make_conditional_expression_t<ElseExpression>;
  using expression_type =
    if_then_else_<if_expression, then_expression, else_expression>;
  return expression_type(std::forward<IfExpression>(if_),
                         std::forward<ThenExpression>(then_),
                         std::forward<ElseExpression>(else_));
}

} // namespace deferred

#endif
