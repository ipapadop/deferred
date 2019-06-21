/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_WHILE_HPP
#define DEFERRED_WHILE_HPP

#include <type_traits>
#include <utility>

#include "expression.hpp"
#include "type_traits/is_constant_expression.hpp"

namespace deferred {

namespace detail {

template<typename T>
constexpr bool evaluate(T&& t)
{
  if constexpr (is_deferred_v<decltype(t)>)
  {
    return evaluate(std::forward<T>(t)());
  }
  else if constexpr (std::is_invocable_v<decltype(t)>)
  {
    return std::forward<T>(t)();
  }
  else
  {
    return t;
  }
}

} // namespace detail

/**
 * Deferred while loop that evaluates @p BodyExpression while @p
 * ConditionExpression evaluates to @c true.
 */
template<typename ConditionExpression, typename BodyExpression>
class while_expression : private std::tuple<ConditionExpression, BodyExpression>
{
private:
  using subexpression_types = std::tuple<ConditionExpression, BodyExpression>;

public:
  using constant_expression =
    std::conjunction<is_constant_expression<ConditionExpression>,
                     is_constant_expression<BodyExpression>>;

  template<typename Condition, typename Body>
  constexpr explicit while_expression(Condition&& condition, Body&& body) :
    std::tuple<ConditionExpression, BodyExpression>(
      std::forward<Condition>(condition),
      std::forward<Body>(body))
  {}

  constexpr void operator()() const
  {
    while (detail::evaluate(
      std::get<0>(static_cast<subexpression_types const&>(*this))))
    {
      std::get<1>(static_cast<subexpression_types const&>(*this))();
    }
  }

  constexpr void operator()()
  {
    while (
      detail::evaluate(std::get<0>(static_cast<subexpression_types&>(*this))))
    {
      std::get<1>(static_cast<subexpression_types&>(*this))();
    }
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/**
 *
 * Example:
 * @code
 * auto var = variable<int>();
 * auto ex = switch_(var,
 *                   default_("unknown"),
 *                   case_(10,
 *                         [] { return "10"; }),
 *                   case_([] { return foo(); },
 *                         [] { return "result of foo"; }));
 * @endcode
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
