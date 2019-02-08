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

/**
 * Holds a constant value.
 */
template<typename T>
class constant_
{
  T m_t;

public:
  /**
   * Constructs a constant_ from @p u.
   */
  template<typename U>
  constexpr constant_(U&& u)
    : m_t(std::forward<U>(u))
  {}

  /**
   * Returns the stored value.
   */
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

/**
 * Creates a new @ref constant_ from @p t.
 */
template<typename T>
constexpr auto constant(T&& t)
{
  return constant_<T>(std::forward<T>(t));
}


/**
 * @brief Checks if @p T is a @ref constant_.
 */
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

} // namespace deferred

#endif

