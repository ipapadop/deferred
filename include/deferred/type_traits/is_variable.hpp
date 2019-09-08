/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TYPE_TRAITS_IS_VARIABLE_HPP
#define DEFERRED_TYPE_TRAITS_IS_VARIABLE_HPP

#include <type_traits>

namespace deferred {

template<typename>
class variable_;

namespace detail {

template<typename>
struct is_variable : public std::false_type
{};

template<typename T>
struct is_variable<variable_<T>> : public std::true_type
{};

} // namespace detail

/// @brief Checks if @p T is a @ref variable_.
template<typename T>
struct is_variable : public detail::is_variable<std::remove_cv_t<T>>
{};

/// Alias for @c is_variable::type.
template<typename T>
using is_variable_t = typename is_variable<T>::type;

/// Alias for @c is_variable::value.
template<typename T>
inline constexpr bool is_variable_v = is_variable<T>::value;

} // namespace deferred

#endif
