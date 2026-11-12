// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_LOGICAL_HPP
#define DEFERRED_LOGICAL_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/eval_mixin.hpp"
#include "detail/is_nothrow_visitable.hpp"
#include "detail/no_unique_address.hpp"
#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

namespace detail {

/**
 * @brief Whether an operator can be overloaded for @p Left and @p Right at all.
 *
 * An overloaded operator needs an operand of class or enumeration type. Checking this
 * first keeps the overload lookups below from being attempted on fundamental types,
 * where naming <tt>operator&&</tt> considers the deferred operators themselves and
 * Clang 18 rejects that as an error rather than a failed requirement.
 *
 * @tparam Left Type of the left operand.
 * @tparam Right Type of the right operand.
 */
template<typename Left, typename Right>
concept Overloadable =
  std::is_class_v<Left> || std::is_enum_v<Left> || std::is_class_v<Right> || std::is_enum_v<Right>;

/** @brief Whether @p Left and @p Right have an overloaded @c && of their own. */
template<typename Left, typename Right>
concept OverloadsConjunction =
  Overloadable<Left, Right>
  && (requires(Left left, Right right) { operator&&(left, right); }
      || requires(Left left, Right right) { left.operator&&(right); });

/** @brief Whether @p Left and @p Right have an overloaded @c || of their own. */
template<typename Left, typename Right>
concept OverloadsDisjunction =
  Overloadable<Left, Right>
  && (requires(Left left, Right right) { operator||(left, right); }
      || requires(Left left, Right right) { left.operator||(right); });

/** @brief Tag selecting a short-circuiting @c && . */
struct conjunction
{
  /// @brief The left operand decides the result when it converts to this.
  static constexpr bool short_circuits_on = false;

  /// @brief Whether @p Left and @p Right have an overloaded @c && of their own.
  template<typename Left, typename Right>
  static constexpr bool overloaded = OverloadsConjunction<Left, Right>;
};

/** @brief Tag selecting a short-circuiting @c || . */
struct disjunction
{
  /// @copydoc conjunction::short_circuits_on
  static constexpr bool short_circuits_on = true;

  /// @brief Whether @p Left and @p Right have an overloaded @c || of their own.
  template<typename Left, typename Right>
  static constexpr bool overloaded = OverloadsDisjunction<Left, Right>;
};

/**
 * @brief Whether the result of evaluating @p T can be tested as a condition.
 *
 * Short-circuiting means testing the left operand on its own, which is only possible for
 * a result that converts to @c bool. An operand that instead overloads @c && or @c ||
 * elementwise -- @c std::valarray, and expression types like it -- keeps the eager,
 * operand-wise meaning, which is also what C++ does for an overloaded @c && .
 *
 * @tparam T Expression argument type.
 */
template<typename T>
concept BooleanTestable =
  requires(evaluated_result_t<make_deferred_t<T>&> value) { static_cast<bool>(value); };

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
concept ShortCircuits = BooleanTestable<T> && BooleanTestable<U>
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
template<typename Operation, Deferred LeftExpression, Deferred RightExpression>
class logical_expression :
  public detail::eval_mixin<logical_expression<Operation, LeftExpression, RightExpression>>
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
