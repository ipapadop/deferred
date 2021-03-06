/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */


#ifndef DEFERRED_OPERATORS_HPP
#define DEFERRED_OPERATORS_HPP

#include <functional>
#include <type_traits>
#include <utility>

#include "invoke.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator+(T&& t, U&& u)
{
  return invoke(std::plus<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator-(T&& t, U&& u)
{
  return invoke(std::minus<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, std::enable_if_t<is_deferred_v<std::decay_t<T>>>* = nullptr>
constexpr auto operator+(T&& t)
{
  return invoke([](auto&& x) { return +std::forward<decltype(x)>(x); }, std::forward<T>(t));
}

template<typename T, std::enable_if_t<is_deferred_v<std::decay_t<T>>>* = nullptr>
constexpr auto operator-(T&& t)
{
  return invoke(std::negate<>{}, std::forward<T>(t));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator*(T&& t, U&& u)
{
  return invoke(std::multiplies<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator/(T&& t, U&& u)
{
  return invoke(std::divides<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator%(T&& t, U&& u)
{
  return invoke(std::modulus<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, std::enable_if_t<is_deferred_v<std::decay_t<T>>>* = nullptr>
constexpr auto operator++(T&& t)
{
  return invoke([](auto&& x) { return ++std::forward<decltype(x)>(x); }, std::forward<T>(t));
}

template<typename T, std::enable_if_t<is_deferred_v<std::decay_t<T>>>* = nullptr>
constexpr auto operator++(T&& t, int)
{
  return invoke([](auto&& x) { return std::forward<decltype(x)>(x)++; }, std::forward<T>(t));
}

template<typename T, std::enable_if_t<is_deferred_v<std::decay_t<T>>>* = nullptr>
constexpr auto operator--(T&& t)
{
  return invoke([](auto&& x) { return --std::forward<decltype(x)>(x); }, std::forward<T>(t));
}

template<typename T, std::enable_if_t<is_deferred_v<std::decay_t<T>>>* = nullptr>
constexpr auto operator--(T&& t, int)
{
  return invoke([](auto&& x) { return std::forward<decltype(x)>(x)--; }, std::forward<T>(t));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator==(T&& t, U&& u)
{
  return invoke(std::equal_to<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator!=(T&& t, U&& u)
{
  return invoke(std::not_equal_to<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator>(T&& t, U&& u)
{
  return invoke(std::greater<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator<(T&& t, U&& u)
{
  return invoke(std::less<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator>=(T&& t, U&& u)
{
  return invoke(std::greater_equal<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator<=(T&& t, U&& u)
{
  return invoke(std::less_equal<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator&&(T&& t, U&& u)
{
  return invoke(std::logical_and<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator||(T&& t, U&& u)
{
  return invoke(std::logical_or<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, std::enable_if_t<is_deferred_v<std::decay_t<T>>>* = nullptr>
constexpr auto operator!(T&& t)
{
  return invoke(std::logical_not<>{}, std::forward<T>(t));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator&(T&& t, U&& u)
{
  return invoke(std::bit_and<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator|(T&& t, U&& u)
{
  return invoke(std::bit_or<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator^(T&& t, U&& u)
{
  return invoke(std::bit_xor<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, std::enable_if_t<is_deferred_v<std::decay_t<T>>>* = nullptr>
constexpr auto operator~(T&& t)
{
  return invoke(std::bit_not<>{}, std::forward<T>(t));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator<<(T&& t, U&& u)
{
  return invoke(
    [](auto&& x, auto&& y) { return std::forward<decltype(x)>(x) << std::forward<decltype(y)>(y); },
    std::forward<T>(t),
    std::forward<U>(u));
}

template<typename T,
         typename U,
         std::enable_if_t<any_deferred_v<std::decay_t<T>, std::decay_t<U>>>* = nullptr>
constexpr auto operator>>(T&& t, U&& u)
{
  return invoke(
    [](auto&& x, auto&& y) { return std::forward<decltype(x)>(x) >> std::forward<decltype(y)>(y); },
    std::forward<T>(t),
    std::forward<U>(u));
}

} // namespace deferred

#endif
