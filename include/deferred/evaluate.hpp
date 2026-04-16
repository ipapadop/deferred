// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_EVALUATE_HPP
#define DEFERRED_EVALUATE_HPP

#include <type_traits>
#include <utility>

#include "type_traits/is_deferred.hpp"

namespace deferred {

/**
 * @brief Evaluates a deferred expression.
 * @tparam T The type of the expression to evaluate.
 * @param t The expression to evaluate.
 * @return The result of evaluating the expression.
 */
template<typename T>
constexpr auto evaluate(T&& t)
{
  if constexpr (Deferred<T>)
  {
    if constexpr (std::is_void_v<decltype(std::forward<T>(t)())>)
    {
      std::forward<T>(t)();
    }
    else
    {
      return evaluate(std::forward<T>(t)());
    }
  }
  else
  {
    return std::forward<T>(t);
  }
}

/**
 * @brief Recursively evaluates @p t until a non-callable is returned.
 * @copydoc evaluate(T&&)
 */
template<typename T>
constexpr auto recursive_evaluate(T&& t)
{
  if constexpr (std::is_invocable_v<T>)
  {
    return recursive_evaluate(std::forward<T>(t)());
  }
  else
  {
    return std::forward<T>(t);
  }
}

} // namespace deferred

#endif
