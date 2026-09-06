// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_DEDUCE_HOMOGENIZED_TYPE_HPP
#define DEFERRED_DETAIL_DEDUCE_HOMOGENIZED_TYPE_HPP

#include <tuple>
#include <type_traits>
#include <variant>

#include "map_void.hpp"
#include "unique_list.hpp"

namespace deferred::detail {

/**
 * @brief Deduce the result type of @p Ts...
 *
 * - If all @p Ts... are the same, return that type.
 * - Otherwise, return a @c std::variant of the unique types in @p Ts...,
 *   mapping @c void to @c std::monostate.
 *
 * @tparam Ts Types to deduce.
 * @return A @c std::type_identity containing the deduced result type.
 */
template<typename... Ts>
consteval auto deduce_homogenized_type()
{
  using unique_mapped = typename unique_list<map_void_t<Ts>...>::type;

  if constexpr (std::tuple_size_v<unique_mapped> == 1)
  {
    return std::type_identity<std::tuple_element_t<0, unique_mapped>>{};
  }
  else
  {
    return []<typename... Us>(std::tuple<Us...>*) {
      return std::type_identity<std::variant<Us...>>{};
    }((unique_mapped*)nullptr);
  }
}

/**
 * @brief Specialization for no types.
 */
template<>
consteval auto deduce_homogenized_type<>()
{
  return std::type_identity<void>{};
}

} // namespace deferred::detail

#endif
