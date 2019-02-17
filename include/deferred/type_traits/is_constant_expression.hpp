/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TYPE_TRAITS_IS_CONSTANT_EXPRESSION_HPP
#define DEFERRED_TYPE_TRAITS_IS_CONSTANT_EXPRESSION_HPP

#include <type_traits>

namespace deferred {

namespace detail {

// Checks if T is a constant expression.
template<typename T, typename = std::void_t<>>
struct is_constant_expression : public std::true_type
{};

// If is_constant_expression is defined, then it is a deferred data type that is
// potentially a constant expression
template<typename T>
struct is_constant_expression<T, std::void_t<typename T::constant_expression>> :
  public T::constant_expression
{};

} // namespace detail

/// Checks if @p T is a constant expression.
template<typename T>
struct is_constant_expression :
  public detail::is_constant_expression<std::decay_t<T>>
{};

/// Alias for @c is_constant::type.
template<typename T>
using is_constant_expression_t = typename is_constant_expression<T>::type;

/// Alias for @c is_constant::value.
template<typename T>
inline constexpr bool is_constant_expression_v =
  is_constant_expression<T>::value;

} // namespace deferred

#endif
