// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_EXPRESSION_HPP
#define DEFERRED_EXPRESSION_HPP

#include <cstddef>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "constant.hpp"
#include "detail/apply_evaluated.hpp"
#include "detail/immediate_assignment.hpp"
#include "detail/is_nothrow_visitable.hpp"
#include "detail/no_unique_address.hpp"
#include "detail/tuple_references.hpp"
#include "detail/visit_children.hpp"
#include "type_traits/expression_visitor.hpp"
#include "type_traits/is_constant_expression.hpp"
#include "type_traits/is_deferred.hpp"

#include "node_ref.hpp"

namespace deferred {

/**
 * @brief Deferred expression that is composed of an operator @p Operator applied to
 * subexpressions @p Expressions....
 *
 * @tparam Operator Type of the operator.
 * @tparam Expressions Types of the subexpressions.
 */
template<typename Operator, deferred_expression... Expressions>
class expression_
{
public:
  using operator_type       = Operator;
  using subexpression_types = std::tuple<Expressions...>;

private:
  DEFERRED_NO_UNIQUE_ADDRESS operator_type m_op;
  DEFERRED_NO_UNIQUE_ADDRESS subexpression_types m_expressions;

public:
  /**
   * @brief Constructs an expression.
   * @tparam Op Type of the operator.
   * @tparam Ex Types of the subexpressions.
   * @param op Operator.
   * @param ex Subexpressions.
   */
  template<typename Op, typename... Ex>
    requires(!std::is_same_v<std::remove_cvref_t<Op>, expression_>)
  constexpr explicit expression_(Op&& op, Ex&&... ex) noexcept(
    std::is_nothrow_constructible_v<operator_type, Op&&>
    && std::is_nothrow_constructible_v<subexpression_types, Ex&&...>) :
    m_op(std::forward<Op>(op)), m_expressions(std::forward<Ex>(ex)...)
  { }

  expression_(expression_ const&) = default;
  expression_(expression_&&)      = default;

  ~expression_() = default;

  expression_& operator=(expression_ const&) = delete;
  expression_& operator=(expression_&&)      = delete;

  /**
   * @brief Evaluates the subexpressions and applies the operator to the results.
   * @return The result of the operator.
   */
  constexpr decltype(auto)
  operator()() const noexcept(noexcept(detail::apply_evaluated(m_op, m_expressions)))
  {
    return detail::apply_evaluated(m_op, m_expressions);
  }

  /// @copydoc operator()() const
  constexpr decltype(auto)
  operator()() noexcept(noexcept(detail::apply_evaluated(m_op, m_expressions)))
  {
    return detail::apply_evaluated(m_op, m_expressions);
  }

  /**
   * @brief Evaluates the expression.
   *
   * A named spelling of <tt>operator()</tt>.
   *
   * @return The result of calling the expression.
   */
  constexpr decltype(auto) eval() const noexcept(noexcept((*this)()))
  {
    return (*this)();
  }

  /// @copydoc expression_::eval() const
  constexpr decltype(auto) eval() noexcept(noexcept((*this)()))
  {
    return (*this)();
  }

  /// @brief Returns the operator.
  [[nodiscard]] constexpr operator_type const& operator_() const noexcept
  {
    return m_op;
  }

  /**
   * @brief Returns references to the subexpressions.
   * @return A @c std::tuple of references, in @c subexpression_types order.
   */
  [[nodiscard]] constexpr auto subexpressions() const noexcept
  {
    return detail::tuple_references(m_expressions);
  }

  /**
   * @brief Visits the expression with a visitor.
   * @tparam Visitor The type of the visitor.
   * @param v The visitor.
   * @param nesting The nesting level.
   */
  template<typename Visitor>
    requires expression_visitor<Visitor, expression_>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, expression_, subexpression_types>)
  {
    v(*this, nesting);
    detail::visit_children(m_expressions, v, nesting + 1);
  }
};

namespace detail {

/**
 * @brief Whether evaluating @p Node needs it to be non-const.
 *
 * True for an @ref expression_ whose callable cannot be called as @c const , such as a
 * lambda declared @c mutable : calling it changes state it holds.
 *
 * @tparam Node Node type.
 */
template<typename Node>
inline constexpr bool callable_needs_mutable = false;

template<typename Operator, typename... Expressions>
inline constexpr bool callable_needs_mutable<expression_<Operator, Expressions...>> =
  !std::is_invocable_v<Operator const&, decltype(std::declval<Expressions&>()())...>;

// A callable with no members has no state to change, whatever its operator() is
// qualified with. Every operator object of the library is empty, so this also spares
// most expressions the question.
template<typename Operator, typename... Expressions>
  requires std::is_empty_v<Operator>
inline constexpr bool callable_needs_mutable<expression_<Operator, Expressions...>> = false;

template<typename Node>
consteval bool owns_state();

// Whether any of the node types in a tuple owns state.
template<typename... Children>
consteval bool any_owns_state(std::type_identity<std::tuple<Children...>>)
{
  return (owns_state<Children>() || ...);
}

/**
 * @brief Whether @p Node , or a node it stores, has state of its own.
 *
 * A copy of such a node is a second, independent state, so an expression does not take
 * one silently. A node has state of its own when it declares @c mutable_state and
 * does not merely refer to state (@ref refers_to_state), when its callable can only
 * be called as non-const, or when it stores a node that has.
 *
 * @tparam Node Node type, without reference or cv-qualifier.
 * @return @c true when copying @p Node copies state.
 */
template<typename Node>
consteval bool owns_state()
{
  if constexpr ((has_mutable_state<Node> && !refers_to_state<Node>)
                || any_owns_state(std::type_identity<typename Node::subexpression_types>{}))
  {
    return true;
  }
  else
  {
    // Asked last, and with the subexpressions evaluated as they are stored rather than
    // as const: a subexpression may be callable only as non-const without owning state.
    return callable_needs_mutable<Node>;
  }
}

/**
 * @brief Deduce the deferred type for @p T , by the rules of @ref make_deferred_t.
 *
 * An operand that cannot be stored is rejected here, with a message that names the
 * fix, because every construct stores its operands through this function.
 *
 * @tparam T Type to transform.
 * @return A @c std::type_identity containing the deduced deferred type.
 */
template<typename T>
consteval auto deduce_deferred_type()
{
  using U = std::decay_t<T>;
  static_assert(!std::is_same_v<U, immediate_assignment>,
                "`v = value` assigns immediately and is not an expression; to assign when "
                "the expression is evaluated, write `v = deferred::constant(value)`");
  if constexpr (is_variable<U>)
  {
    // The condition is spelled with T so that it is checked only when this branch is
    // taken.
    static_assert(std::is_lvalue_reference_v<T>,
                  "a temporary variable cannot be used in an expression, which would refer "
                  "to it after it is destroyed: give the variable a name");
    return std::type_identity<node_ref<std::remove_reference_t<T>>>{};
  }
  else if constexpr (is_reference_to_deferred<U>)
  {
    // std::ref(node) or std::cref(node).
    return std::type_identity<node_ref<typename U::type>>{};
  }
  else if constexpr (deferred_expression<T>)
  {
    // A named operand would be copied into the expression. The condition of each
    // assertion repeats that of its branch so that it is checked only when the branch is
    // taken.
    if constexpr (!std::is_lvalue_reference_v<T>)
    {
    }
    else if constexpr (owns_state<U>())
    {
      static_assert(!owns_state<U>(),
                    "this operand has state of its own, and an expression would take a copy "
                    "of it: write std::ref(x) for the expression to refer to x, or "
                    "std::move(x) to give x to the expression");
    }
    else
    {
      static_assert(std::is_copy_constructible_v<U>,
                    "this operand cannot be copied into the expression: write std::move(x) "
                    "to give x to the expression, or std::ref(x) for the expression to "
                    "refer to x");
    }
    return std::type_identity<U>{};
  }
  else if constexpr (std::is_invocable_v<U>)
  {
    return std::type_identity<expression_<U>>{};
  }
  else
  {
    return std::type_identity<constant<U>>{};
  }
}

} // namespace detail

/**
 * @brief Transforms @p T into a @c deferred type.
 *
 * - If @p T is an lvalue reference to a @ref variable , it is transformed to a
 *   @ref variable_ref , or to a @ref variable_cref when the variable is @c const , so
 *   the expression refers to the variable.
 * - If @p T is any other @c deferred type, it is transformed to that type without
 *   reference or cv-qualifier, so the expression owns a copy of the operand.
 * - If @p T is a callable type, it is transformed to an @ref expression_.
 * - If @p T is not a callable type, it is transformed to an @ref constant.
 *
 * The result is never a reference.
 *
 * @tparam T Type to transform.
 */
template<typename T>
using make_deferred_t = typename decltype(detail::deduce_deferred_type<T>())::type;

} // namespace deferred

#endif
