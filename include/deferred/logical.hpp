// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_LOGICAL_HPP
#define DEFERRED_LOGICAL_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "concepts.hpp"
#include "detail/is_nothrow_visitable.hpp"
#include "detail/no_unique_address.hpp"
#include "evaluate.hpp"
#include "expression.hpp"
#include "type_traits/expression_visitor.hpp"

namespace deferred {

namespace detail {

/** @brief Whether @p Left and @p Right have an overloaded @c && of their own. */
template<typename Left, typename Right>
concept overloads_conjunction = requires(Left left, Right right) { operator&&(left, right); }
                                || requires(Left left, Right right) { left.operator&&(right); };

/** @brief Whether @p Left and @p Right have an overloaded @c || of their own. */
template<typename Left, typename Right>
concept overloads_disjunction = requires(Left left, Right right) { operator||(left, right); }
                                || requires(Left left, Right right) { left.operator||(right); };

/** @brief Tag selecting a short-circuiting @c && . */
struct conjunction
{
  /// @brief The left operand decides the result when it converts to this.
  static constexpr bool short_circuits_on = false;

  /// @brief Whether @p Left and @p Right have an overloaded @c && of their own.
  template<typename Left, typename Right>
  static constexpr bool overloaded = overloads_conjunction<Left, Right>;
};

/** @brief Tag selecting a short-circuiting @c || . */
struct disjunction
{
  /// @copydoc conjunction::short_circuits_on
  static constexpr bool short_circuits_on = true;

  /// @brief Whether @p Left and @p Right have an overloaded @c || of their own.
  template<typename Left, typename Right>
  static constexpr bool overloaded = overloads_disjunction<Left, Right>;
};

/**
 * @brief Whether @p Operation on @p T and @p U short-circuits.
 *
 * It does when both results can be tested as conditions and their types do not overload
 * the operator. A type that converts to @c bool and also has its own @c && or @c || --
 * a three-valued boolean, for example -- keeps its operator, which C++ evaluates
 * eagerly, and the result type that operator yields.
 *
 * @tparam Operation @ref conjunction or @ref disjunction.
 * @tparam T Left expression argument type.
 * @tparam U Right expression argument type.
 */
template<typename Operation, typename T, typename U>
concept short_circuits =
  condition<T> && condition<U>
  && !Operation::template overloaded<evaluated_result_t<make_deferred_t<T>&>,
                                     evaluated_result_t<make_deferred_t<U>&>>;

} // namespace detail

/**
 * @brief Deferred short-circuiting @c && or @c || .
 *
 * The right operand is evaluated only when the left one does not already decide the
 * result, matching the built-in operators. This is a node of its own rather than an
 * @ref expression_ over a function object because an @ref expression_ evaluates every
 * subexpression as an argument of one call, which cannot express "evaluate this one
 * only sometimes".
 *
 * @tparam Operation @ref detail::conjunction or @ref detail::disjunction.
 * @tparam LeftExpression Type of the left operand.
 * @tparam RightExpression Type of the right operand.
 */
template<typename Operation, deferred_expression LeftExpression, deferred_expression RightExpression>
class logical_expression
{
public:
  using subexpression_types = std::tuple<LeftExpression, RightExpression>;

  /// @brief Result type of the logical expression.
  using result_type = bool;

private:
  DEFERRED_NO_UNIQUE_ADDRESS LeftExpression m_left;
  DEFERRED_NO_UNIQUE_ADDRESS RightExpression m_right;

  template<typename Self>
  static constexpr bool evaluate_impl(Self& self)
  {
    if (static_cast<bool>(evaluate_ref(self.m_left)) == Operation::short_circuits_on)
    {
      return Operation::short_circuits_on;
    }
    return static_cast<bool>(evaluate_ref(self.m_right));
  }

public:
  /**
   * @brief Constructs a logical_expression.
   * @tparam Left Type of the left operand.
   * @tparam Right Type of the right operand.
   * @param left Left operand.
   * @param right Right operand.
   */
  template<typename Left, typename Right>
  constexpr explicit logical_expression(Left&& left, Right&& right) noexcept(
    std::is_nothrow_constructible_v<LeftExpression, Left&&>
    && std::is_nothrow_constructible_v<RightExpression, Right&&>) :
    m_left(std::forward<Left>(left)), m_right(std::forward<Right>(right))
  { }

  /**
   * @brief Evaluates the operands, stopping as soon as the result is decided.
   * @return The result of the operation.
   */
  constexpr bool operator()() const noexcept(noexcept(static_cast<bool>(evaluate_ref(m_left)))
                                             && noexcept(static_cast<bool>(evaluate_ref(m_right))))
  {
    return evaluate_impl(*this);
  }

  /// @copydoc logical_expression::operator()() const
  constexpr bool operator()() noexcept(noexcept(static_cast<bool>(evaluate_ref(m_left)))
                                       && noexcept(static_cast<bool>(evaluate_ref(m_right))))
  {
    return evaluate_impl(*this);
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

  /// @copydoc logical_expression::eval() const
  constexpr decltype(auto) eval() noexcept(noexcept((*this)()))
  {
    return (*this)();
  }

  /**
   * @brief Returns references to the subexpressions.
   * @return A @c std::tuple of references, in @c subexpression_types order.
   */
  [[nodiscard]] constexpr auto subexpressions() const noexcept
  {
    return std::forward_as_tuple(m_left, m_right);
  }

  /**
   * @brief Visits the expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
    requires expression_visitor<Visitor, logical_expression>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, logical_expression, subexpression_types>)
  {
    v(*this, nesting);
    m_left.visit(v, nesting + 1);
    m_right.visit(v, nesting + 1);
  }
};

namespace detail {

/**
 * @brief Short-circuiting expression type produced for an operation and its operands.
 * @tparam Operation @ref conjunction or @ref disjunction.
 * @tparam Left Left operand argument type.
 * @tparam Right Right operand argument type.
 */
template<typename Operation, typename Left, typename Right>
using logical_expression_t =
  logical_expression<Operation, make_deferred_t<Left>, make_deferred_t<Right>>;

/**
 * @brief Checks whether building a short-circuiting expression cannot throw.
 * @tparam Operation @ref conjunction or @ref disjunction.
 * @tparam Left Left operand argument type.
 * @tparam Right Right operand argument type.
 * @return @c true when storing the operands cannot throw.
 */
template<typename Operation, typename Left, typename Right>
consteval bool logical_is_nothrow()
{
  return std::
    is_nothrow_constructible_v<logical_expression_t<Operation, Left, Right>, Left&&, Right&&>;
}

} // namespace detail

} // namespace deferred

#endif
