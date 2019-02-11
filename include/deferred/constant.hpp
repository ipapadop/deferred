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

#include "type_traits/is_constant.hpp"
#include "type_traits/is_constant_expression.hpp"
#include "type_traits/is_expression.hpp"

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

  constant_& operator=(constant_ const&) = delete;
  constant_& operator=(constant_&&) = delete;

  /// Returns the stored value.
  constexpr decltype(auto) operator()() const noexcept
  {
    if constexpr (is_expression_v<std::decay_t<T>>)
    {
      return m_t();
    }
    else
    {
      return m_t;
    }
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/**
 * Creates a new @ref constant_ from @p t.
 * 
 * @warning This function accepts non @c deferred objects and @ref constant_.
 */
template<typename T,
         std::enable_if_t<is_constant_expression_v<std::decay_t<T>>>* = nullptr>
constexpr auto constant(T&& t)
{
  using result_type = std::conditional_t<is_constant_v<std::decay_t<T>>, T, constant_<T>>;
  return result_type(std::forward<T>(t));
}

} // namespace deferred

#endif
