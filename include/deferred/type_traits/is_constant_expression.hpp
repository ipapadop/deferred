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

#include "is_deferred.hpp"
#include "is_constant.hpp"
#include "is_expression.hpp"

namespace deferred
{

/// Checks if @p T is a constant.
template<typename T, typename = void>
struct is_constant_expression
  : public std::false_type
{};

namespace detail
{

// Check is std::tuple<T...> consists of types that satisfy is_constant.
template<typename... T>
struct is_constant_expression_tuple;

template<typename... T>
struct is_constant_expression_tuple<std::tuple<T...>>
  : public std::conjunction<is_constant<T>...>
{};

} // namespace detail

template<typename T>
struct is_constant_expression<T,
                              std::enable_if_t<!is_deferred_v<T>>>
  : public std::true_type
{};

template<typename T>
struct is_constant_expression<T,
                              std::enable_if_t<is_constant_v<T>>>
  : public std::true_type
{};

template<typename T>
struct is_constant_expression<T,
                              std::enable_if_t<is_expression_v<T>>>
  : public detail::is_constant_expression_tuple<typename T::subexpression_types>
{};

/// Alias for @c is_constant::type.
template<typename T>
using is_constant_expression_t = typename is_constant_expression<T>::type;

/// Alias for @c is_constant::value.
template<typename T>
inline constexpr bool is_constant_expression_v = is_constant_expression<T>::value;

} // namespace deferred

#endif
