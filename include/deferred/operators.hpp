/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */


#ifndef DEFERRED_OPERATORS_HPP
#define DEFERRED_OPERATORS_HPP

#include <functional>
#include <ostream>

#include "constant.hpp"
#include "expression.hpp"
#include "type_traits.hpp"

namespace deferred
{

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator+(T&& t, U&& u)
{
  return make_expression(std::plus<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator-(T&& t, U&& u)
{
  return make_expression(std::minus<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename = std::enable_if_t<is_deferred_datatype_t<std::decay_t<T>>::value>>
constexpr auto operator+(T&& t)
{
  return make_expression([](auto x) { return +x; }, std::forward<T>(t));
}

template<typename T,
         typename = std::enable_if_t<is_deferred_datatype_t<std::decay_t<T>>::value>>
constexpr auto operator-(T&& t)
{
  return make_expression(std::negate<>{}, std::forward<T>(t));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator*(T&& t, U&& u)
{
  return make_expression(std::multiplies<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator/(T&& t, U&& u)
{
  return make_expression(std::divides<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator%(T&& t, U&& u)
{
  return make_expression(std::modulus<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator==(T&& t, U&& u)
{
  return make_expression(std::equal_to<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator!=(T&& t, U&& u)
{
  return make_expression(std::not_equal_to<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator>(T&& t, U&& u)
{
  return make_expression(std::greater<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator<(T&& t, U&& u)
{
  return make_expression(std::less<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator>=(T&& t, U&& u)
{
  return make_expression(std::greater_equal<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator<=(T&& t, U&& u)
{
  return make_expression(std::less_equal<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator&&(T&& t, U&& u)
{
  return make_expression(std::logical_and<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator||(T&& t, U&& u)
{
  return make_expression(std::logical_or<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename = std::enable_if_t<is_deferred_datatype_t<std::decay_t<T>>::value>>
constexpr auto operator!(T&& t)
{
  return make_expression(std::logical_not<>{}, std::forward<T>(t));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator&(T&& t, U&& u)
{
  return make_expression(std::bit_and<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator|(T&& t, U&& u)
{
  return make_expression(std::bit_or<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T, typename U,
         typename = std::enable_if_t<any_deferred_datatypes_t<T, U>::value>>
constexpr auto operator^(T&& t, U&& u)
{
  return make_expression(std::bit_xor<>{}, std::forward<T>(t), std::forward<U>(u));
}

template<typename T,
         typename = std::enable_if_t<is_deferred_datatype_t<std::decay_t<T>>::value>>
constexpr auto operator~(T&& t)
{
  return make_expression(std::bit_not<>{}, std::forward<T>(t));
}

template<typename T,
         typename = std::enable_if_t<is_deferred_datatype<std::decay_t<T>>::value>>
std::ostream& operator<<(std::ostream& os, T const& t)
{
  return os << t();
}

} // namespace deferred

#endif
