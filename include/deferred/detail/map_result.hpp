// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_MAP_RESULT_HPP
#define DEFERRED_MAP_RESULT_HPP

#include <functional>
#include <type_traits>
#include <utility>
#include <variant>

namespace deferred::detail {

/**
 * @brief Maps the result of an evaluation to the target result type.
 * @tparam Result Target result type.
 * @tparam F Type of the function that evaluates the expression.
 * @param f Function that evaluates the expression.
 * @return Mapped result.
 */
template<typename Result, typename F>
constexpr decltype(auto) map_result(F&& f)
{
  if constexpr (std::is_void_v<Result>)
  {
    static_cast<void>(std::invoke(std::forward<F>(f)));
  }
  else if constexpr (std::is_void_v<std::invoke_result_t<F>>)
  {
    std::invoke(std::forward<F>(f));
    return std::monostate{};
  }
  else
  {
    return std::invoke(std::forward<F>(f));
  }
}

} // namespace deferred::detail

#endif
