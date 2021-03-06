/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
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

/// Returns the result of @p t().
template<typename T>
constexpr decltype(auto) evaluate(T&& t)
{
  if constexpr (is_deferred_v<std::remove_reference_t<T>>)
  {
    if constexpr (std::is_same_v<decltype(std::forward<T>(t)()), void>)
    {
      std::forward<T>(t)();
    }
    else
    {
      auto v = evaluate(std::forward<T>(t)());
      return v;
    }
  }
  else
  {
    return std::forward<T>(t);
  }
}

/// Recursively evaluates @p t until a non-callable is returned.
template<typename T>
constexpr decltype(auto) recursive_evaluate(T&& t)
{
  if constexpr (std::is_invocable_v<T>)
  {
    auto v = recursive_evaluate(std::forward<T>(t)());
    return v;
  }
  else
  {
    return std::forward<T>(t);
  }
}

} // namespace deferred

#endif
