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

#include <utility>

#include "constant.hpp"
#include "expression.hpp"
#include "type_traits/make_deferred.hpp"

namespace deferred
{

/**
 * TODO
 */
template<typename IfExpression, typename ThenExpression, typename ElseExpression>
class if_then_else_
  : private std::tuple<IfExpression, ThenExpression, ElseExpression>
{
public:
  template<typename IfEx, typename ThenEx, typename ElseEx>
  constexpr explicit if_then_else_(IfEx&& if_, ThenEx&& then_, ElseEx&& else_)
    : std::tuple<IfExpression, ThenExpression, ElseExpression>(
        std::forward<IfEx>(if_),
        std::forward<ThenEx>(then_),
        std::forward<ElseEx>(else_))
  {}

  constexpr decltype(auto) operator()() const
  {
    if (std::get<0>(*this)())
    {
      return std::get<1>(*this)();
    }
    else
    {
      return std::get<2>(*this)();
    }
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/**
 * TODO
 */
template<typename IfExpression, typename ThenExpression, typename ElseExpression>
constexpr auto if_then_else(IfExpression&& if_, ThenExpression&& then_, ElseExpression&& else_)
{
  using if_expression   = make_deferred_t<IfExpression>;
  using then_expression = make_deferred_t<ThenExpression>;
  using else_expression = make_deferred_t<ElseExpression>;
  using result_type     = if_then_else_<if_expression, then_expression, else_expression>;
  return result_type(std::forward<IfExpression>(if_),
                     std::forward<ThenExpression>(then_),
                     std::forward<ElseExpression>(else_));
}

} // namespace deferred

#endif
