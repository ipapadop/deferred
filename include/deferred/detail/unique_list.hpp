// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_UNIQUE_LIST_HPP
#define DEFERRED_DETAIL_UNIQUE_LIST_HPP

#include <tuple>
#include <type_traits>
#include <utility>

namespace deferred::detail {

/**
 * @brief Metafunction to create a unique list of types.
 * @tparam Ts Input types.
 */
template<typename... Ts>
struct unique_list
{
  using type = std::tuple<>;
};

/**
 * @brief Recursive case for @ref unique_list.
 * @tparam T Current type.
 * @tparam Ts Remaining types.
 */
template<typename T, typename... Ts>
struct unique_list<T, Ts...>
{
  /**
   * @brief Tail of the unique list.
   */
  using tail = typename unique_list<Ts...>::type;

  /**
   * @brief Resulting type list after potentially prepending @p T.
   */
  using type = std::conditional_t<(std::is_same_v<T, Ts> || ...),
                                  tail,
                                  decltype(std::tuple_cat(std::declval<std::tuple<T>>(),
                                                          std::declval<tail>()))>;
};

} // namespace deferred::detail

#endif
