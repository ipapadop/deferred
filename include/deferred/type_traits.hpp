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

/**
 * Forms the logical conjunction of the type traits @p B... .
 */
template<typename... B>
struct conjunction
  : public std::is_same<detail::bool_pack<true, B::value...>, detail::bool_pack<B::value..., true>>
{};


template<typename...>
struct disjunction
  : public std::false_type
{};

template<typename B>
struct disjunction<B>
  : public B
{};

/**
 * Forms the logical disjunction of the type traits <tt>B,Bn...</tt>.
 */
template<typename B, typename... Bn>
struct disjunction<B, Bn...>
  : public std::conditional_t<bool(B::value), B, disjunction<Bn...>>
{};

/**
 * Forms the logical negation of type trait @p B.
 */
template<typename B>
struct negation
  : public std::integral_constant<bool, !bool(B::value)>
{};

} // namespace deferred

#endif

