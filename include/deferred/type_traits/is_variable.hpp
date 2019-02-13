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

#include "is_deferred.hpp"

namespace deferred
{

template<typename>
class variable_;

/// @brief Checks if @p T is a @ref variable_.
template<typename...>
struct is_variable
  : public std::false_type
{};

template<typename T>
struct is_variable<variable_<T>>
  : public std::true_type
{};

/// Alias for @c is_variable::type.
template<typename T>
using is_variable_t = typename is_variable<T>::type;

/// Alias for @c is_variable::value.
template<typename T>
inline constexpr bool is_variable_v = is_variable<T>::value;


template<typename T>
struct is_deferred<variable_<T>>
  : public std::true_type
{};

} // namespace deferred

#endif
