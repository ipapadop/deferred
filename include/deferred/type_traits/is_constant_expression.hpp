// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_TYPE_TRAITS_IS_CONSTANT_EXPRESSION_HPP
#define DEFERRED_TYPE_TRAITS_IS_CONSTANT_EXPRESSION_HPP

#include <concepts>
#include <tuple>
#include <type_traits>

#include "is_deferred.hpp"

namespace deferred {

template<typename T>
struct is_constant_expression;

/**
 * @brief Concept for expressions whose own stored value can change between evaluations.
 *
 * An expression opts out of @c is_constant_expression by declaring
 * <tt>static constexpr bool mutable_state = true;</tt>. Only the node itself is opted
 * out; its subexpressions are still examined.
 *
 * @tparam T The type to check against the concept.
 */
template<typename T>
concept has_mutable_state = requires {
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

// Checks if the operator of T, when it has one, is a constant expression.
template<typename T>
consteval bool operator_is_constant()
{
  if constexpr (requires { typename T::operator_type; })
  {
    return deferred::is_constant_expression<typename T::operator_type>::value;
  }
  else
  {
    return true;
  }
}

// If subexpression_types is defined, then it is a deferred data type that is
// potentially a constant expression: it is one when it does not hold mutable state of
// its own, its operator does not either, and none of its subexpressions does.
template<typename T>
struct is_constant_expression<T, std::void_t<typename T::subexpression_types>> :
  public std::bool_constant<!deferred::has_mutable_state<T> && operator_is_constant<T>()
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

/**
 * @brief Concept for a @c deferred expression that is a constant expression.
 *
 * The concept form of @ref is_constant_expression_v , restricted to @c deferred
 * expressions: the trait is also @c true for an empty type that is not an expression.
 *
 * @tparam T The type to check against the concept.
 */
template<typename T>
concept constant_expression = deferred_expression<T> && is_constant_expression_v<T>;

} // namespace deferred

#endif
