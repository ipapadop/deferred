// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_NODE_REF_HPP
#define DEFERRED_NODE_REF_HPP

#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "type_traits/expression_visitor.hpp"

namespace deferred {

template<typename T>
class variable;

/**
 * @brief Refers to a node that lives outside the expression.
 *
 * An expression stores its operands by value. Where it must use the caller's own
 * node -- a @ref variable it reads and writes, or any node passed with @c std::ref or
 * @c std::cref -- it stores a @c node_ref to it instead. The node must outlive every
 * expression that refers to it.
 *
 * Evaluating a @c node_ref evaluates the node, and a visitor is shown the node. A
 * @c node_ref to a @c const node evaluates it as @c const.
 *
 * It is created by the library; there is no need to name it. @ref variable_ref and
 * @ref variable_cref name the two forms a variable takes.
 *
 * @tparam Node Type of the node referred to, @c const when access is read-only.
 */
template<typename Node>
class node_ref
{
public:
  /// @brief Type of the node referred to.
  using node_type = Node;

  using subexpression_types = std::tuple<>;

  /// @brief The node referred to can change between evaluations.
  static constexpr bool mutable_state = true;

private:
  Node* m_node;

public:
  /**
   * @brief Constructs a reference to @p node.
   * @param node Node to refer to.
   */
  constexpr explicit node_ref(Node& node) noexcept : m_node(std::addressof(node))
  { }

  /// @brief A temporary node would not outlive the reference.
  node_ref(Node&&) = delete;

  node_ref(node_ref const&) = default;

  node_ref& operator=(node_ref const&) = delete;

  /// @brief Evaluates the node referred to.
  constexpr decltype(auto) operator()() const noexcept(noexcept((*m_node)()))
  {
    return (*m_node)();
  }

  /**
   * @brief Returns what <tt>get()</tt> of the node referred to returns.
   *
   * Present when the node has it, as a @ref variable and a @ref constant do.
   */
  constexpr decltype(auto) get() const noexcept(noexcept(m_node->get()))
    requires requires(Node& node) { node.get(); }
  {
    return m_node->get();
  }

  /**
   * @brief Returns references to the subexpressions.
   * @return An empty @c std::tuple.
   */
  [[nodiscard]] constexpr auto subexpressions() const noexcept
  {
    return std::tuple<>{};
  }

  /**
   * @brief Visits the node referred to with a visitor.
   *
   * The visitor is called with the node, not with the reference, so a visitor sees
   * the same nodes whichever way an expression stores them.
   *
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
    requires expression_visitor<Visitor, node_ref>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(noexcept(std::as_const(*m_node).visit(v, nesting)))
  {
    std::as_const(*m_node).visit(v, nesting);
  }
};

/**
 * @brief What an expression stores for a named @ref variable.
 * @tparam T Type of the value of the variable.
 */
template<typename T>
using variable_ref = node_ref<variable<T>>;

/**
 * @brief What an expression stores for a named @ref variable that is @c const.
 * @tparam T Type of the value of the variable.
 */
template<typename T>
using variable_cref = node_ref<variable<T> const>;

namespace detail {

/**
 * @brief Whether the state @p Node marks with @c mutable_state is held somewhere else.
 *
 * A node that owns state must not be copied without being asked, but a node that
 * only refers to state -- a @ref node_ref , or an assignment, which changes its
 * target -- can be.
 *
 * @tparam Node Node type.
 */
template<typename Node>
inline constexpr bool refers_to_state = false;

template<typename Node>
inline constexpr bool refers_to_state<node_ref<Node>> = true;

// A visitor is called with the node a node_ref refers to.
template<typename Visitor, typename Node>
struct visits_tree<Visitor, node_ref<Node>> : visits_tree<Visitor, std::remove_const_t<Node>>
{ };

/// @brief Whether @p T is a @ref variable.
template<typename T>
inline constexpr bool is_variable = false;

template<typename T>
inline constexpr bool is_variable<variable<T>> = true;

} // namespace detail

} // namespace deferred

#endif
