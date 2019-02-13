/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TYPE_TRAITS_IS_DEFERRED_HPP
#define DEFERRED_TYPE_TRAITS_IS_DEFERRED_HPP

#include <type_traits>

namespace deferred
{

/// Checks if @p T is a type from @c deferred.
template<typename T>
struct is_deferred
  : public std::false_type
{};

/// Alias for @c is_deferred_datatype::type.
template<typename T>
using is_deferred_t = typename is_deferred<T>::type;

/// Alias for @c is_deferred::value.
template<typename T>
inline constexpr bool is_deferred_v = is_deferred<T>::value;


/// Checks if any of @p T... satisfies @ref is_deferred.
template<typename... T>
using any_deferred_t = std::disjunction<is_deferred_t<std::decay_t<T>>...>;

/// Alias for @c any_deferred_t::value.
template<typename... T>
inline constexpr bool any_deferred_v = any_deferred_t<T...>::value;

} // namespace deferred

#endif
