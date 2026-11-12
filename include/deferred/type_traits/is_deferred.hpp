// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_TYPE_TRAITS_IS_DEFERRED_HPP
#define DEFERRED_TYPE_TRAITS_IS_DEFERRED_HPP

#include <functional>
#include <type_traits>

namespace deferred {

/**
 * @brief Checks if @p T has a nested type @c subexpression_types.
 * @tparam T The type to check against the concept.
 */
template<typename T>
concept has_subexpression_types = requires { typename T::subexpression_types; };

/**
 * @brief Concept for types that are deferred expressions.
 * @tparam T The type to check against the concept.
 */
template<typename T>
concept deferred_expression = has_subexpression_types<std::remove_cvref_t<T>>;

/**
 * @brief Checks if @p T is a type in @c deferred namespace.
 * @tparam T The type to check.
 */
template<typename T>
struct is_deferred : public std::bool_constant<deferred_expression<T>>
{ };

/**
 * @brief Alias for @c is_deferred::type.
 * @tparam T The type to check.
 */
template<typename T>
using is_deferred_t = typename is_deferred<T>::type;

/**
 * @brief Alias for @c is_deferred::value.
 * @tparam T The type to check.
 */
template<typename T>
inline constexpr bool is_deferred_v = is_deferred<T>::value;

namespace detail {

// Whether T is a std::reference_wrapper to a deferred expression.
template<typename T>
inline constexpr bool is_reference_to_deferred = false;

template<typename Node>
inline constexpr bool is_reference_to_deferred<std::reference_wrapper<Node>> =
  deferred_expression<Node>;

} // namespace detail

/**
 * @brief Concept for an operand that makes an operator build a @c deferred expression.
 *
 * Either a @c deferred expression, or a @c std::reference_wrapper to one:
 * <tt>std::ref(x)</tt> and <tt>std::cref(x)</tt> are how an expression refers to a node
 * rather than taking a copy of it.
 *
 * @tparam T The type to check against the concept.
 */
template<typename T>
concept deferred_operand =
  deferred_expression<T> || detail::is_reference_to_deferred<std::remove_cvref_t<T>>;

/**
 * @brief Checks if any of @p T is a @ref deferred_operand.
 * @tparam T The types to check.
 */
template<typename... T>
concept any_deferred = (deferred_operand<T> || ...);

} // namespace deferred

#endif
