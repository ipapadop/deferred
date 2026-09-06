// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_HOMOGENIZED_TYPE_HPP
#define DEFERRED_DETAIL_HOMOGENIZED_TYPE_HPP

#include <type_traits>
#include <variant>

#include "deduce_homogenized_type.hpp"

namespace deferred::detail {

/**
 * @brief Metafunction to deduce the homogenized type of @p Ts..., or a variant if they are not all
 * identical.
 * @tparam Ts Types to deduce.
 *
 * - If all @p Ts... are identical, that type is used.
 * - Otherwise, a @c std::variant of the unique types in @p Ts... is used,
 *   mapping @c void to @c std::monostate.
 */
template<typename... Ts>
struct homogenized_type
{
  using deduced_type = typename decltype(deduce_homogenized_type<Ts...>())::type;
  using type = std::conditional_t<std::is_same_v<deduced_type, std::monostate>, void, deduced_type>;
};

/**
 * @brief Alias for the deduced homogenized type.
 * @tparam Ts Types to deduce.
 */
template<typename... Ts>
using homogenized_type_t = typename homogenized_type<Ts...>::type;

} // namespace deferred::detail

#endif
