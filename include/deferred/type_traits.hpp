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

/// Forms the logical conjunction of the type traits @p B....
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

/// Forms the logical disjunction of the type traits <tt>B,Bn...</tt>.
template<typename B, typename... Bn>
struct disjunction<B, Bn...>
  : public std::conditional_t<bool(B::value), B, disjunction<Bn...>>
{};


/// Forms the logical negation of type trait @p B.
template<typename B>
struct negation
  : public std::integral_constant<bool, !bool(B::value)>
{};


/// Maps sequence of types to @c void.
template<typename...>
using void_t = void;


namespace detail
{

template<typename, typename = void>
struct is_invocable
  : public std::false_type
{};
 
template<typename F, typename... Args>
struct is_invocable<F(Args...), void_t<std::result_of_t<F(Args...)>>>
  : public std::true_type
{};

} // namespace detail
 
template<typename T>
struct is_invocable
  : public detail::is_invocable<T>
{};

template<typename T>
using is_invocable_t = typename is_invocable<T>::type;

/**
 * Checks if @p T is a type from @c deferred.
 */
template<typename T>
struct is_deferred
  : public std::false_type
{};

/// Alias for @c is_deferred_datatype::type.
template<typename T>
using is_deferred_t = typename is_deferred<T>::type;

/**
 * Checks if any of @p T... satisfies @ref is_deferred.
 */
template<typename... T>
using any_deferred_t = disjunction<is_deferred_t<std::decay_t<T>>...>;

} // namespace deferred

#endif

