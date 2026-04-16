// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_TYPE_TRAITS_IS_DEFERRED_HPP
#define DEFERRED_TYPE_TRAITS_IS_DEFERRED_HPP

#include <type_traits>

namespace deferred {

namespace detail {

template<typename T, typename = std::void_t<>>
struct has_subexpression_types : std::false_type
{ };

template<typename T>
struct has_subexpression_types<T, std::void_t<typename T::subexpression_types>> : std::true_type
{ };

} // namespace detail

/// Checks if @p T is a type in @c deferred namespace.
/// @tparam T The type to check.
template<typename T>
struct is_deferred : public detail::has_subexpression_types<std::remove_cv_t<T>>
{ };

/// Alias for @c is_deferred_datatype::type.
/// @tparam T The type to check.
template<typename T>
using is_deferred_t = typename is_deferred<T>::type;

/// Alias for @c is_deferred::value.
/// @tparam T The type to check.
template<typename T>
inline constexpr bool is_deferred_v = is_deferred<T>::value;

/// Concept for types that are deferred expressions.
/// @tparam T The type to check against the concept.
template<typename T>
concept Deferred = is_deferred_v<std::remove_cvref_t<T>>;

/// Checks if any of @p T... satisfies @ref is_deferred.
/// @tparam T A parameter pack of types to check.
template<typename... T>
using any_deferred_t = std::disjunction<is_deferred_t<T>...>;

/// Alias for @c any_deferred_t::value.
/// @tparam T A parameter pack of types to check.
template<typename... T>
inline constexpr bool any_deferred_v = any_deferred_t<T...>::value;

/// Concept for when at least one type in a set is a deferred expression.
/// @tparam T A parameter pack of types to check.
template<typename... T>
concept AnyDeferred = (Deferred<T> || ...);

} // namespace deferred

#endif
