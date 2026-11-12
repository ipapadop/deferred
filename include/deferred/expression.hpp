// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_EXPRESSION_HPP
#define DEFERRED_EXPRESSION_HPP

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
#include "type_traits/is_deferred.hpp"

#include "variable_ref.hpp"

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
  operator()() const& noexcept(noexcept(detail::apply_evaluated(m_op, m_expressions)))
  {
    return detail::apply_evaluated(m_op, m_expressions);
  }

  /// @copydoc operator()() const&
  constexpr decltype(auto)
  operator()() & noexcept(noexcept(detail::apply_evaluated(m_op, m_expressions)))
  {
    return detail::apply_evaluated(m_op, m_expressions);
  }

  /**
   * @brief Evaluates a temporary expression.
   *
   * The subexpressions are evaluated as temporaries too. One that owns its value then
   * yields the value rather than a reference to it, so the result cannot refer into
   * this expression, which is about to be destroyed. A @ref variable_ref still yields
   * a reference, because the variable it refers to outlives the expression.
   *
   * @return The result of the operator.
   */
  constexpr decltype(auto)
  operator()() && noexcept(noexcept(detail::apply_evaluated(m_op, std::move(m_expressions))))
  {
    return detail::apply_evaluated(m_op, std::move(m_expressions));
  }

  /**
   * @brief Evaluates the expression.
   *
   * A named spelling of <tt>operator()</tt>.
   *
   * @return The result of calling the expression.
   */
  constexpr decltype(auto) eval() const& noexcept(noexcept((*this)()))
  {
    return (*this)();
  }

  /// @copydoc expression_::eval() const&
  constexpr decltype(auto) eval() & noexcept(noexcept((*this)()))
  {
    return (*this)();
  }

  /// @copydoc expression_::eval() const&
  constexpr decltype(auto) eval() && noexcept(noexcept(std::move(*this)()))
  {
    return std::move(*this)();
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
 * @brief Deduce the deferred type for @p T.
 *
 * - If @p T is an lvalue reference to a @ref variable , it is transformed to a
 *   @ref variable_ref , so the expression refers to the variable.
 * - If @p T is any other @c deferred type, it is transformed to that type without
 *   reference or cv-qualifier, so the expression owns a copy of the operand.
 * - If @p T is a callable type, it is transformed to an @ref expression_.
 * - If @p T is not a callable type, it is transformed to an @ref constant.
 *
 * The result is never a reference.
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
  if constexpr (requires { typename variable_ref_for<T>::type; })
  {
    return variable_ref_for<T>{};
  }
  else if constexpr (deferred_expression<T>)
  {
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
 *   @ref variable_ref , so the expression refers to the variable.
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
