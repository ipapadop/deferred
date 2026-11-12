// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_CONCEPTS_HPP
#define DEFERRED_CONCEPTS_HPP

#include <concepts>
#include <type_traits>
#include <utility>

#include "evaluate.hpp"
#include "expression.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

/**
 * @brief Concept for a @c deferred expression whose result converts to @p Result.
 *
 * The result is that of calling the expression as an lvalue, so a reference result is
 * checked as a reference: a variable satisfies <tt>expression_yielding<int&></tt>. An
 * expression yielding @c void , such as a loop, satisfies it for @c void only.
 *
 * @tparam T The type to check against the concept.
 * @tparam Result The type the result must convert to.
 */
template<typename T, typename Result>
concept expression_yielding =
  deferred_expression<T> && requires(std::remove_reference_t<T>& expression) {
    { expression() } -> std::convertible_to<Result>;
  };

/**
 * @brief Concept for a @c deferred expression whose result is an lvalue.
 *
 * An operation that hands back a reference into its operand -- a subscript, a
 * dereference, an assignment target -- is only safe when the operand yields an lvalue
 * that outlives the call. An operand yielding a prvalue is materialized for the duration
 * of one call and the reference would dangle.
 *
 * @tparam T The type to check against the concept.
 */
template<typename T>
concept lvalue_expression =
  deferred_expression<T>
  && std::is_lvalue_reference_v<decltype(std::declval<make_deferred_t<T>&>()())>;

/**
 * @brief Concept for an argument that can be the condition of @c if_ , a loop, or a
 *        short-circuiting @c && or @c || .
 *
 * The argument is made @c deferred as the construct would make it -- a plain value
 * becomes a constant and a callable an expression -- and its result must convert to
 * @c bool explicitly, as the condition of a built-in @c if does.
 *
 * @tparam T The argument type to check against the concept.
 */
template<typename T>
concept condition =
  requires(detail::evaluated_result_t<make_deferred_t<T>&> value) { static_cast<bool>(value); };

/**
 * @brief Concept for an argument that can be a @c case_ label of a switch on
 *        @p Condition.
 *
 * Both arguments are made @c deferred as @c switch_ and @c case_ would make them. The
 * result of the condition must be comparable to the result of the label with @c == ,
 * giving something that converts to @c bool.
 *
 * @tparam Label The label argument type to check against the concept.
 * @tparam Condition The argument type of the switch condition.
 */
template<typename Label, typename Condition>
concept case_label_for =
  requires(detail::evaluated_result_t<make_deferred_t<Condition>&> const& value,
           make_deferred_t<Label>& label) { static_cast<bool>(value == evaluate_ref(label)); };

} // namespace deferred

#endif
