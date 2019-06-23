/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_EVALUATE_HPP
#define DEFERRED_EVALUATE_HPP

#include <type_traits>
#include <utility>

#include "type_traits/is_deferred.hpp"

namespace deferred {

/// Returns @p t.
template<typename T, std::enable_if_t<!is_deferred_v<T>>* = nullptr>
constexpr decltype(auto) evaluate(T&& t) noexcept
{
  return std::forward<T>(t);
}

/// Returns the result of @p t().
template<typename T, std::enable_if_t<is_deferred_v<T>>* = nullptr>
constexpr auto evaluate(T&& t)
{
  return evaluate(std::forward<T>(t)());
}

/// Returns @p t.
template<typename T>
constexpr void evaluate_void(T&& t) noexcept
{
  if constexpr(is_deferred_v<decltype(std::forward<T>(t)())>)
  {
    evaluate_void(std::forward<T>(t)());
  }
  else
  {
    std::forward<T>(t)();
  }
}

} // namespace deferred

#endif
