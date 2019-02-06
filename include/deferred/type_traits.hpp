/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TYPE_TRAITS_HPP
#define DEFERRED_TYPE_TRAITS_HPP

#include <type_traits>

namespace deferred
{

namespace detail
{

template<bool...>
struct bool_pack
{};

} // namespace detail

template<typename... T>
struct conjunction
  : public std::is_same<detail::bool_pack<true, T::value...>, detail::bool_pack<T::value..., true>>
{};


template<typename...>
struct disjunction
  : public std::false_type
{};

template<typename B>
struct disjunction<B>
  : public B
{};

template<typename B, typename... Bn>
struct disjunction<B, Bn...>
  : public std::conditional_t<bool(B::value), B, disjunction<Bn...>>
{};


template<typename B>
struct negation
  : public std::integral_constant<bool, !bool(B::value)>
{};

} // namespace deferred

#endif

