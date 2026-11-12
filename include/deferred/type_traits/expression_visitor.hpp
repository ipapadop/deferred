// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_TYPE_TRAITS_EXPRESSION_VISITOR_HPP
#define DEFERRED_TYPE_TRAITS_EXPRESSION_VISITOR_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>

#include "is_deferred.hpp"

namespace deferred {

namespace detail {

// Whether Visitor can be called with Node and with every node below it.
template<typename Visitor, typename Node>
struct visits_tree;

// Whether Visitor can be called with every node of the trees rooted at Children.
template<typename Visitor, typename Children>
struct visits_children;

template<typename Visitor, typename... Children>
struct visits_children<Visitor, std::tuple<Children...>> :
  std::bool_constant<(visits_tree<Visitor, std::remove_cvref_t<Children>>::value && ...)>
{ };

template<typename Visitor, typename Node>
struct visits_tree :
  std::bool_constant<std::is_invocable_v<Visitor&, Node const&, std::size_t>
                     && visits_children<Visitor, typename Node::subexpression_types>::value>
{ };

} // namespace detail

/**
 * @brief Concept for a visitor that can traverse the expression @p Node.
 *
 * <tt>node.visit(v)</tt> calls <tt>v(n, nesting)</tt> for @p Node and for every
 * expression below it, where @c n is a @c const reference to the expression and
 * @c nesting a @c std::size_t. A visitor satisfies the concept when all of those calls
 * are valid, so a visitor that cannot take some node of the tree is rejected by
 * @c visit() itself rather than part-way through the traversal.
 *
 * @tparam Visitor The visitor type; a reference type is treated as the type referred to.
 * @tparam Node The expression type to traverse.
 */
template<typename Visitor, typename Node>
concept expression_visitor =
  deferred_expression<Node>
  && detail::visits_tree<std::remove_reference_t<Visitor>, std::remove_cvref_t<Node>>::value;

} // namespace deferred

#endif
