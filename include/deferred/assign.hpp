// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_ASSIGN_HPP
#define DEFERRED_ASSIGN_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/is_nothrow_visitable.hpp"
#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

namespace detail {

/** @brief Implements deferred assignment. */
struct assign_to
{
  /// @brief Assigns @p value to @p target.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) = std::forward<U>(value)))
  {
    return std::forward<T>(target) = std::forward<U>(value);
  }
};

} // namespace detail

/**
 * @brief Deferred assignment of @p ValueExpression to @p TargetExpression.
 *
 * The value expression is evaluated on every evaluation of the assignment, and its
 * result is assigned to the object the target expression yields. Assignment is by
 * value: the target receives a copy of the value the right-hand side evaluates to,
 * exactly as a built-in assignment would, so a later change to the right-hand side
 * does not affect the target.
 *
 * This is a node of its own rather than an @ref expression_ over an assigning
 * operator. An @ref expression_ holds its subexpressions in a @c std::tuple, and
 * instantiating a @c std::tuple that holds a @ref variable_ reference asks whether
 * that reference is assignable -- a question this very operator answers, which would
 * re-enter the expression type while it is still incomplete. Holding the two
 * subexpressions directly keeps that question answerable from the declaration alone.
 *
 * @p Operation selects which assignment is performed: @ref detail::assign_to for a
 * plain assignment, or one of the compound assigning operations in @ref operators.hpp.
 * The target is evaluated once, so a compound assignment does not repeat it.
 *
 * @tparam Operation Type of the assigning operation.
 * @tparam TargetExpression Type of the assignment target expression.
 * @tparam ValueExpression Type of the assigned value expression.
 */
template<typename Operation, Deferred TargetExpression, Deferred ValueExpression>
class assign_expression
{
public:
  /// @brief Assigning mutates its target, so an assignment is never constant.
  static constexpr bool mutable_state = true;

  using expression_types    = std::tuple<TargetExpression, ValueExpression>;
  using subexpression_types = std::tuple<Operation, TargetExpression, ValueExpression>;

private:
  [[no_unique_address]] TargetExpression m_target;
  [[no_unique_address]] ValueExpression m_value;

public:
  /**
   * @brief Constructs an assign_expression.
   * @tparam Target Type of the assignment target expression.
   * @tparam Value Type of the assigned value expression.
   * @param target Assignment target expression.
   * @param value Assigned value expression.
   */
  template<typename Target, typename Value>
  constexpr explicit assign_expression(Target&& target, Value&& value) noexcept(
    std::is_nothrow_constructible_v<TargetExpression, Target&&>
    && std::is_nothrow_constructible_v<ValueExpression, Value&&>) :
    m_target(std::forward<Target>(target)), m_value(std::forward<Value>(value))
  { }

  /**
   * @brief Performs the assignment.
   * @return The result of the assignment, as a built-in assignment would.
   */
  constexpr decltype(auto)
  operator()() const noexcept(noexcept(Operation{}(m_target(), evaluate(m_value))))
  {
    // A built-in assignment sequences its right operand before its left one, but the
    // arguments of a call are only indeterminately sequenced. Evaluating the value into
    // a local first keeps the ordering the built-in operator guarantees.
    auto value = evaluate(m_value);
    return Operation{}(m_target(), std::move(value));
  }

  /// @copydoc assign_expression::operator()() const
  constexpr decltype(auto)
  operator()() noexcept(noexcept(Operation{}(m_target(), evaluate(m_value))))
  {
    auto value = evaluate(m_value);
    return Operation{}(m_target(), std::move(value));
  }

  /**
   * @brief Visits the assignment expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, assign_expression, expression_types>)
  {
    v(*this, nesting);
    m_target.visit(v, nesting + 1);
    m_value.visit(v, nesting + 1);
  }
};

namespace detail {

/**
 * @brief Assignment expression type produced for an operation and its operands.
 * @tparam Operation Type of the assigning operation.
 * @tparam Target Assignment target argument type.
 * @tparam Value Assigned value argument type.
 */
template<typename Operation, typename Target, typename Value>
using assign_expression_t =
  assign_expression<Operation, make_deferred_t<Target>, make_deferred_t<Value>>;

/**
 * @brief Checks whether building a deferred assignment cannot throw.
 * @tparam Operation Type of the assigning operation.
 * @tparam Target Assignment target argument type.
 * @tparam Value Assigned value argument type.
 * @return @c true when storing the operands cannot throw.
 */
template<typename Operation, typename Target, typename Value>
consteval bool assign_is_nothrow()
{
  return std::
    is_nothrow_constructible_v<assign_expression_t<Operation, Target, Value>, Target&&, Value&&>;
}

} // namespace detail

} // namespace deferred

#endif
