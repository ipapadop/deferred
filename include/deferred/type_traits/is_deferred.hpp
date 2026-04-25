// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_TYPE_TRAITS_IS_DEFERRED_HPP
#define DEFERRED_TYPE_TRAITS_IS_DEFERRED_HPP

#include <type_traits>

namespace deferred {

/**
 * @brief Checks if @p T has a nested type @c subexpression_types.
 * @tparam T The type to check against the concept.
 */
template<typename T>
concept HasSubexpressionTypes = requires { typename T::subexpression_types; };

/**
 * @brief Concept for types that are deferred expressions.
 * @tparam T The type to check against the concept.
 */
template<typename T>
concept Deferred = HasSubexpressionTypes<std::remove_cvref_t<T>>;

/**
 * @brief Checks if @p T is a type in @c deferred namespace.
 * @tparam T The type to check.
 */
template<typename T>
struct is_deferred : public std::bool_constant<Deferred<T>>
{ };

/**
 * @brief Alias for @c is_deferred_datatype::type.
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

/**
 * @brief Concept for when at least one type in a set is a deferred expression.
 * @tparam T A parameter pack of types to check.
 */
template<typename... T>
concept AnyDeferred = (Deferred<T> || ...);

} // namespace deferred

#endif
