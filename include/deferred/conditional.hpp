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
private:
  using subexpression_types =
    std::tuple<IfExpression, ThenExpression, ElseExpression>;

public:
  using constant_expression =
    std::conjunction<is_constant_expression<IfExpression>,
                     is_constant_expression<ThenExpression>,
                     is_constant_expression<ElseExpression>>;
  using result_type =
    std::common_type_t<decltype(std::declval<ThenExpression>()()),
                       decltype(std::declval<ElseExpression>()())>;

  template<typename IfEx, typename ThenEx, typename ElseEx>
  constexpr explicit if_then_else_(IfEx&& if_, ThenEx&& then_, ElseEx&& else_) :
    std::tuple<IfExpression, ThenExpression, ElseExpression>(
      std::forward<IfEx>(if_),
      std::forward<ThenEx>(then_),
      std::forward<ElseEx>(else_))
  {}

  constexpr result_type operator()() const
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

/**
 * Creates a new deferred conditional that evaluates @p then_ if @p if_
 * evaluates to @c true, otherwise it evaluates @p else_.
 *
 * The result type of <tt>if_then_else(...)()</tt> is the @c std::common_type of
 * the result types of @p then_ and @p else_.
 *
 * Example:
 * @code
 * auto v = variable<bool>();
 * auto ex = if_then_else(v, constant(42), constant(10));
 * v = true;
 * assert(v() == 42);
 * @endcode
 */
template<typename IfExpression, typename ThenExpression, typename ElseExpression>
constexpr auto
if_then_else(IfExpression&& if_, ThenExpression&& then_, ElseExpression&& else_)
{
  using if_expression   = make_expression_t<IfExpression>;
  using then_expression = make_expression_t<ThenExpression>;
  using else_expression = make_expression_t<ElseExpression>;
  using expression_type =
    if_then_else_<if_expression, then_expression, else_expression>;
  return expression_type(std::forward<IfExpression>(if_),
                         std::forward<ThenExpression>(then_),
                         std::forward<ElseExpression>(else_));
}

} // namespace deferred

#endif
