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

template<typename...>
struct conjunction
  : public std::true_type
{};

template<typename B>
struct conjunction<B>
  : public B
{};

template<typename B, typename... Bn>
struct conjunction<B, Bn...>
  : public std::conditional_t<bool(B::value), conjunction<Bn...>, B>
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

