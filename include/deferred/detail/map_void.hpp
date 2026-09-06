// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_MAP_VOID_HPP
#define DEFERRED_DETAIL_MAP_VOID_HPP

#include <type_traits>
#include <variant>

namespace deferred::detail {

/**
 * @brief Maps @c void to @c std::monostate for use in @c std::variant.
 * @tparam T Type to map.
 */
template<typename T>
using map_void_t = std::conditional_t<std::is_void_v<T>, std::monostate, T>;

} // namespace deferred::detail

#endif
