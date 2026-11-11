// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_ASSIGN_HPP
#define DEFERRED_ASSIGN_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/is_nothrow_visitable.hpp"
#include "detail/no_unique_address.hpp"
#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

namespace detail {

/**
 * @brief Whether @p Operation can assign @p Value to what evaluating @p Target yields.
 *
 * @c YieldsLvalue keeps the target's result alive; this additionally requires that the
 * assignment itself is well formed, so a @c const target is rejected at the call rather
 * than deep inside the node.
 *
 * @tparam Operation Type of the assigning operation.
 * @tparam Target Assignment target argument type.
 * @tparam Value Assigned value argument type.
 */
template<typename Operation, typename Target, typename Value>
concept AssignableWith =
  YieldsLvalue<Target>
  && requires(make_deferred_t<Target>& target, evaluated_result_t<make_deferred_t<Value>&> value) {
       Operation{}(target(), std::move(value));
     };

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
 * This is a node of its own rather than an @ref expression_ over an assigning operator,
 * for two independent reasons. An @ref expression_ evaluates its subexpressions as the
 * arguments of one call, which are only indeterminately sequenced, so it cannot give the
 * right-before-left ordering a built-in assignment guarantees. And it holds them in a
 * @c std::tuple, so instantiating one that holds a @ref variable_ reference asks whether
 * that reference is assignable -- the question this operator answers -- re-entering the
 * type while it is still incomplete. Both were reproduced on GCC and Clang.
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

  using subexpression_types = std::tuple<TargetExpression, ValueExpression>;

private:
  DEFERRED_NO_UNIQUE_ADDRESS TargetExpression m_target;
  DEFERRED_NO_UNIQUE_ADDRESS ValueExpression m_value;

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
  operator()() const noexcept(noexcept(Operation{}(m_target(), evaluate_ref(m_value))))
  {
    // A built-in assignment sequences its right operand before its left one, but the
    // arguments of a call are only indeterminately sequenced. Evaluating the value into
    // a local first keeps the ordering the built-in operator guarantees.
    decltype(auto) value = evaluate_ref(m_value);
    return Operation{}(m_target(), value);
  }

  /// @copydoc assign_expression::operator()() const
  constexpr decltype(auto)
  operator()() noexcept(noexcept(Operation{}(m_target(), evaluate_ref(m_value))))
  {
    decltype(auto) value = evaluate_ref(m_value);
    return Operation{}(m_target(), value);
  }

  /**
   * @brief Visits the assignment expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, assign_expression, subexpression_types>)
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

/**
 * @brief Creates a deferred assignment of @p value to @p target.
 *
 * The spelling for targets that are not a @ref variable_ -- anything whose evaluation
 * yields an lvalue, such as a member accessed through @ref invoke():
 * @code
 * auto ex = assign(invoke(&counter::value, &c), constant(42));
 * @endcode
 *
 * @tparam Target Type of the assignment target expression.
 * @tparam Value Type of the assigned value expression.
 * @param target Assignment target; must be a deferred expression yielding an lvalue, which
 * @ref detail::YieldsLvalue enforces.
 * @param value Expression whose value is assigned.
 * @return An @ref assign_expression assigning to @p target when evaluated.
 */
template<typename Target, typename Value>
  requires detail::AssignableWith<detail::assign_to, Target, Value>
[[nodiscard]] constexpr auto assign(Target&& target, Value&& value) //
  noexcept(detail::assign_is_nothrow<detail::assign_to, Target, Value>())
{
  return detail::assign_expression_t<detail::assign_to, Target, Value>(std::forward<Target>(target),
                                                                       std::forward<Value>(value));
}

} // namespace deferred

#endif
