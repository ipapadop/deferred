/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_CONSTANT_HPP
#define DEFERRED_CONSTANT_HPP

#include <type_traits>
#include <utility>

#include "type_traits.hpp"

namespace deferred
{

/// Holds a constant value.
template<typename T>
class constant_
{
  T m_t;

public:
  /// Constructs a constant_ from @p u.
  template<typename U,
           std::enable_if_t<std::is_convertible_v<U, T>>* = nullptr>
  constexpr explicit constant_(U&& u)
    : m_t(std::forward<U>(u))
  {}

  constant_(constant_ const&) = default;
  constant_(constant_&&) = default;

  /// Returns the stored value.
  constexpr const T& operator()() const noexcept
  {
    return m_t;
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/// Checks if @p T is a @ref constant_.
template<typename>
struct is_constant
  : public std::false_type
{};

template<typename T>
struct is_constant<constant_<T>>
  : public std::true_type
{};

/// Alias for @c is_constant::type.
template<typename T>
using is_constant_t = typename is_constant<T>::type;

/// Alias for @c is_constant::value.
template<typename T>
inline constexpr bool is_constant_v = is_constant<T>::value;

template<typename T>
struct is_deferred<constant_<T>>
  : public std::true_type
{};

namespace detail
{

// Checks if T is a value or another constant.
template<typename T>
inline constexpr bool is_constant_or_value_v =
  std::disjunction_v<std::negation<is_deferred_t<T>>, is_constant_t<T>>;

} // namespace detail

/**
 * Creates a new @ref constant_ from @p t.
 * 
 * @warning This function accepts non @c deferred objects and @ref constant_.
 */
template<typename T,
         std::enable_if_t<detail::is_constant_or_value_v<std::decay_t<T>>>* = nullptr>
constexpr auto constant(T&& t)
{
  using result_type = std::conditional_t<is_constant_v<std::decay_t<T>>, T, constant_<T>>;
  return result_type(std::forward<T>(t));
}

} // namespace deferred

#endif
