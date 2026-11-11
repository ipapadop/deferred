// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_TUPLE_REFERENCES_HPP
#define DEFERRED_DETAIL_TUPLE_REFERENCES_HPP

#include <tuple>

namespace deferred::detail {

/**
 * @brief Returns a tuple of lvalue references to the elements of @p t.
 *
 * An element held by value is referred to as @c const, as @p t is; an element that is
 * itself a reference keeps the type it has.
 *
 * @tparam Tuple Type of the tuple.
 * @param t Tuple whose elements are referred to.
 * @return A @c std::tuple of references, in the order of the elements of @p t.
 */
template<typename Tuple>
[[nodiscard]] constexpr auto tuple_references(Tuple const& t) noexcept
{
  return std::apply([](auto&... element) noexcept { return std::forward_as_tuple(element...); }, t);
}

} // namespace deferred::detail

#endif
