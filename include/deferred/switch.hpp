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

template<typename LabelExpression, typename CaseExpression>
auto case_(LabelExpression&& label, CaseExpression&& body)
{
  using body_expression = make_expression_t<CaseExpression>;
  return body_expression(std::forward<CaseExpression>(body));
}

template<typename Expression>
auto default_(Expression&& ex)
{
  using expression = make_expression_t<Expression>;
  return expression(std::forward<Expression>(ex));
}

template<typename SwitchExpression, typename... Expressions>
auto switch_(SwitchExpression&& sw, Expressions&&... ex)
{
  using switch_expression = make_expression_t<SwitchExpression>;
  return switch_expression(std::forward<SwitchExpression>(sw));
}

} // namespace deferred

#endif
