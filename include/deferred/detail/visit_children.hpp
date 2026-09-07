// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_VISIT_CHILDREN_HPP
#define DEFERRED_DETAIL_VISIT_CHILDREN_HPP

#include <cstddef>
#include <tuple>

namespace deferred::detail {

/**
 * @brief Visits every child expression held in a tuple.
 *
 * Runtime counterpart of @ref is_nothrow_visitable; both describe the same
 * traversal, so they are kept in step.
 *
 * @tparam Children Tuple of child expressions.
 * @tparam Visitor Type of the visitor.
 * @param children Tuple holding the child expressions.
 * @param v Visitor.
 * @param nesting Nesting level to report for the children.
 */
template<typename Children, typename Visitor>
constexpr void visit_children(Children const& children, Visitor& v, std::size_t nesting)
{
  std::apply([&](auto const&... child) { (child.visit(v, nesting), ...); }, children);
}

} // namespace deferred::detail

#endif
