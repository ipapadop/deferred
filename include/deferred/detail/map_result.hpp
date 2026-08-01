// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_MAP_RESULT_HPP
#define DEFERRED_DETAIL_MAP_RESULT_HPP

#include <type_traits>
#include <utility>
#include <variant>

namespace deferred::detail {

/**
 * @brief Maps the result of an evaluation to the target result type.
 * @tparam Result Target result type.
 * @tparam T Type of the evaluated expression.
 * @param t Evaluated expression.
 * @return Mapped result.
 */
template<typename Result, typename T>
constexpr decltype(auto) map_result(T&& t)
{
  if constexpr (std::is_void_v<Result>)
  {
    static_cast<void>(t);
  }
  else if constexpr (std::is_void_v<T>)
  {
    static_cast<void>(t);
    return std::monostate{};
  }
  else
  {
    return std::forward<T>(t);
  }
}

} // namespace deferred::detail

#endif
