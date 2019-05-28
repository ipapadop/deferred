/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_SWITCH_HPP
#define DEFERRED_SWITCH_HPP

#include <type_traits>
#include <utility>

#include "constant.hpp"
#include "expression.hpp"
#include "type_traits/is_constant_expression.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

namespace detail {

// Transforms T into an expression_ if it is not a deferred data type for use
// with if_then_else_.
template<typename T>
using make_switch_expression_t = std::conditional_t<
  is_deferred_v<T>,
  T,
  std::conditional_t<std::is_invocable_v<T>, make_expression_t<T>, constant_<T>>>;

} // namespace detail

template<typename LabelExpression, typename CaseExpression>
auto case_(LabelExpression&& label, CaseExpression&& body)
{
    using body_expression = detail::make_switch_expression_t<CaseExpression>;
    return body_expression(std::forward<CaseExpression>(body));
}

template<typename Expression>
auto default_(Expression&& ex)
{
    using expression = detail::make_switch_expression_t<Expression>;
    return expression(std::forward<Expression>(ex));
}

template<typename SwitchExpression, typename... Expressions>
auto switch_(SwitchExpression&& sw, Expressions&&... ex)
{
    using switch_expression = detail::make_switch_expression_t<SwitchExpression>;
    return switch_expression(std::forward<SwitchExpression>(sw));
}

} // namespace deferred

#endif
