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

namespace deferred
{

namespace detail
{

template<typename T, typename = std::void_t<>>
struct all_sub_expressions_constant
  : public std::false_type
{};

template<typename T>
struct all_sub_expressions_constant<T, std::void_t<typename T::subexpression_types>>
  : public std::true_type
{};

} // namespace detail

/// Checks if @p T is a constant.
template<typename T>
struct is_constant_expression
  : public std::disjunction<
      is_constant_t<T>,
      std::negation<is_deferred_t<T>>,
      detail::all_sub_expressions_constant<T>>
{};

/// Alias for @c is_constant::type.
template<typename T>
using is_constant_expression_t = typename is_constant_expression<T>::type;

/// Alias for @c is_constant::value.
template<typename T>
inline constexpr bool is_constant_expression_v = is_constant_expression<T>::value;

} // namespace deferred

#endif
