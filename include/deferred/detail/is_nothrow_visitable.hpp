// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_IS_NOTHROW_VISITABLE_HPP
#define DEFERRED_DETAIL_IS_NOTHROW_VISITABLE_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace deferred::detail {

/**
 * @brief Describes whether visiting an expression and all its children cannot throw.
 * @tparam Visitor Visitor type.
 * @tparam Expression Visited expression type.
 * @tparam Children Tuple of child expression types.
 */
template<typename Visitor, typename Expression, typename Children>
struct is_nothrow_visitable;

/**
 * @brief Specializes visitor checking for a tuple of child expression types.
 */
template<typename Visitor, typename Expression, typename... Children>
struct is_nothrow_visitable<Visitor, Expression, std::tuple<Children...>> :
  std::bool_constant<std::is_nothrow_invocable_v<Visitor&, Expression const&, std::size_t>
                     && (noexcept(std::declval<Children const&>().visit(std::declval<Visitor&>(),
                                                                        std::size_t{}))
                         && ...)>
{ };

/**
 * @brief Whether visiting an expression and all its children cannot throw.
 * @tparam Visitor Visitor type.
 * @tparam Expression Visited expression type.
 * @tparam Children Tuple of child expression types.
 */
template<typename Visitor, typename Expression, typename Children>
inline constexpr bool is_nothrow_visitable_v =
  is_nothrow_visitable<Visitor, Expression, Children>::value;

} // namespace deferred::detail

#endif
