// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_TYPE_TRAITS_IS_CONSTANT_EXPRESSION_HPP
#define DEFERRED_TYPE_TRAITS_IS_CONSTANT_EXPRESSION_HPP

#include <concepts>
#include <tuple>
#include <type_traits>

namespace deferred {

template<typename T>
struct is_constant_expression;

/**
 * @brief Concept for expressions whose own stored value can change between evaluations.
 *
 * An expression opts out of @ref is_constant_expression by declaring
 * <tt>static constexpr bool mutable_state = true;</tt>. Only the node itself is opted
 * out; its subexpressions are still examined.
 *
 * @tparam T The type to check against the concept.
 */
template<typename T>
concept HasMutableState = requires {
  { T::mutable_state } -> std::convertible_to<bool>;
} && T::mutable_state;

namespace detail {

// Checks if T is a constant expression.
template<typename T, typename = std::void_t<>>
struct is_constant_expression : public std::is_empty<T>::type
{ };

// Matches the tuple of subexpressions for a deferred type
template<typename... T>
struct is_constant_expression<std::tuple<T...>> :
  public std::conjunction<deferred::is_constant_expression<T>...>
{ };

// If subexpression_types is defined, then it is a deferred data type that is
// potentially a constant expression: it is one when it does not hold mutable state of
// its own and none of its subexpressions does either.
template<typename T>
struct is_constant_expression<T, std::void_t<typename T::subexpression_types>> :
  public std::bool_constant<!deferred::HasMutableState<T>
                            && is_constant_expression<typename T::subexpression_types>::value>
{ };

} // namespace detail

/**
 * @brief Checks if @p T is a constant expression.
 * @tparam T The type to check.
 */
template<typename T>
struct is_constant_expression : public detail::is_constant_expression<std::decay_t<T>>
{ };

/**
 * @brief Alias for @c is_constant_expression::type.
 * @tparam T The type to check.
 */
template<typename T>
using is_constant_expression_t = typename is_constant_expression<T>::type;

/**
 * @brief Alias for @c is_constant_expression::value.
 * @tparam T The type to check.
 */
template<typename T>
inline constexpr bool is_constant_expression_v = is_constant_expression<T>::value;

} // namespace deferred

#endif
