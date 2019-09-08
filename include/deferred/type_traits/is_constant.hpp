/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TYPE_TRAITS_IS_CONSTANT_HPP
#define DEFERRED_TYPE_TRAITS_IS_CONSTANT_HPP

#include <type_traits>

namespace deferred {

template<typename T>
class constant_;

namespace detail {

template<typename>
struct is_constant : public std::false_type {};

template<typename T>
struct is_constant<constant_<T>> : public std::true_type {};

} // namespace detail

/// Checks if @p T is a @ref constant_.
template<typename T>
struct is_constant : public detail::is_constant<std::remove_cv_t<T>> {};

/// Alias for @c is_constant::type.
template<typename T>
using is_constant_t = typename is_constant<T>::type;

/// Alias for @c is_constant::value.
template<typename T>
inline constexpr bool is_constant_v = is_constant<T>::value;

} // namespace deferred

#endif
