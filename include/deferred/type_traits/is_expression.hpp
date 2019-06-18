/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TYPE_TRAITS_IS_EXPRESSION_HPP
#define DEFERRED_TYPE_TRAITS_IS_EXPRESSION_HPP

#include <type_traits>

namespace deferred {

template<typename, typename...>
class expression_;

template<typename, typename, typename>
class if_then_else_expression;

namespace detail {

template<typename...>
struct is_expression : public std::false_type {};

template<typename... T>
struct is_expression<expression_<T...>> : public std::true_type {};

template<typename T, typename U, typename V>
struct is_expression<if_then_else_expression<T, U, V>> : public std::true_type {};

} // namespace detail

/// Checks if @p T is an @ref expression_.
template<typename T>
struct is_expression : public detail::is_expression<std::decay_t<T>> {};

/// Alias for @c is_expression::type.
template<typename T>
using is_expression_t = typename is_expression<T>::type;

/// Alias for @c is_expression::value.
template<typename T>
inline constexpr bool is_expression_v = is_expression<T>::value;

} // namespace deferred

#endif
