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

#include <tuple>
#include <type_traits>

namespace deferred {

template<typename T>
struct is_constant_expression;

namespace detail {

// Checks if T is a constant expression.
template<typename T, typename = std::void_t<>>
struct is_constant_expression : public std::is_empty<T>::type
{};

// Matches the tuple of subexpressions for a deferred type
template<typename... T>
struct is_constant_expression<std::tuple<T...>> :
  public std::conjunction<deferred::is_constant_expression<T>...>
{};

// If subexpression_types is defined, then it is a deferred data type that is
// potentially a constant expression
template<typename T>
struct is_constant_expression<T, std::void_t<typename T::subexpression_types>> :
  public is_constant_expression<typename T::subexpression_types>
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
