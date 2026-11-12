// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DO_WHILE_HPP
#define DEFERRED_DO_WHILE_HPP

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

/**
 * @brief Deferred do-while loop that evaluates @p BodyExpression once and then again
 * while @p ConditionExpression evaluates to @c true.
 *
 * @tparam BodyExpression Type of the body expression.
 * @tparam ConditionExpression Type of the condition expression.
 */
template<deferred_expression BodyExpression, deferred_expression ConditionExpression>
class do_while_expression
{
public:
  using subexpression_types = std::tuple<BodyExpression, ConditionExpression>;

  /// @brief Result type of the loop.
  using result_type = void;

private:
  DEFERRED_NO_UNIQUE_ADDRESS BodyExpression m_body;
  DEFERRED_NO_UNIQUE_ADDRESS ConditionExpression m_condition;

public:
  /**
   * @brief Constructs a do_while_expression.
   * @tparam Body Type of the body expression.
   * @tparam Condition Type of the condition expression.
   * @param body Body expression.
   * @param condition Condition expression.
   */
  template<typename Body, typename Condition>
  constexpr explicit do_while_expression(Body&& body, Condition&& condition) noexcept(
    std::is_nothrow_constructible_v<BodyExpression, Body&&>
    && std::is_nothrow_constructible_v<ConditionExpression, Condition&&>) :
    m_body(std::forward<Body>(body)), m_condition(std::forward<Condition>(condition))
  { }

  /// @brief Evaluates the do-while loop.
  constexpr void
  operator()() const noexcept(noexcept(evaluate_ref(m_body))
                              && noexcept(static_cast<bool>(evaluate_ref(m_condition))))
  {
    do
    {
      evaluate_ref(m_body);
    }
    while (evaluate_ref(m_condition));
  }

  /// @copydoc do_while_expression::operator()() const
  constexpr void operator()() noexcept(noexcept(evaluate_ref(m_body))
                                       && noexcept(static_cast<bool>(evaluate_ref(m_condition))))
  {
    do
    {
      evaluate_ref(m_body);
    }
    while (evaluate_ref(m_condition));
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

  /// @copydoc do_while_expression::eval() const
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
    return std::forward_as_tuple(m_body, m_condition);
  }

  /**
   * @brief Visits the do-while expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
    requires expression_visitor<Visitor, do_while_expression>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, do_while_expression, subexpression_types>)
  {
    v(*this, nesting);
    m_body.visit(v, nesting + 1);
    m_condition.visit(v, nesting + 1);
  }
};

/**
 * @brief Do-while loop waiting for its condition expression.
 * @tparam BodyExpression Type of the body expression.
 */
template<deferred_expression BodyExpression>
class do_while_builder
{
  DEFERRED_NO_UNIQUE_ADDRESS BodyExpression m_body;

public:
  /**
   * @brief Constructs a do_while_builder.
   * @tparam Body Type of the body expression.
   * @param body Body expression awaiting a condition expression.
   */
  template<typename Body>
    requires(!std::is_same_v<std::remove_cvref_t<Body>, do_while_builder>)
  constexpr explicit do_while_builder(Body&& body) noexcept(
    std::is_nothrow_constructible_v<BodyExpression, Body&&>) : m_body(std::forward<Body>(body))
  { }

  /**
   * @brief Completes the loop with its condition expression.
   * @tparam ConditionExpression Type of the condition expression.
   * @param condition Condition expression.
   * @return A @ref do_while_expression capturing the body and condition.
   */
  template<condition ConditionExpression>
  [[nodiscard]] constexpr auto while_(ConditionExpression&& condition) && //
    noexcept(std::is_nothrow_constructible_v<
             do_while_expression<BodyExpression, make_deferred_t<ConditionExpression>>,
             BodyExpression&&,
             ConditionExpression&&>)
  {
    using condition_expression = make_deferred_t<ConditionExpression>;
    return do_while_expression<BodyExpression, condition_expression>(
      std::forward<BodyExpression>(m_body),
      std::forward<ConditionExpression>(condition));
  }

  /// @copydoc while_
  template<condition ConditionExpression>
  [[nodiscard]] constexpr auto while_(ConditionExpression&& condition) const& //
    noexcept(std::is_nothrow_constructible_v<
             do_while_expression<BodyExpression, make_deferred_t<ConditionExpression>>,
             BodyExpression const&,
             ConditionExpression&&>)
  {
    using condition_expression = make_deferred_t<ConditionExpression>;
    return do_while_expression<BodyExpression, condition_expression>(
      m_body,
      std::forward<ConditionExpression>(condition));
  }
};

/**
 * @brief Starts a @c deferred do-while loop whose @p body runs before the condition is
 * checked, and then again while it is @c true.
 *
 * Example:
 * @code
 * auto ex = do_(body).while_(cond);
 * @endcode
 *
 * @tparam BodyExpression Type of the body expression.
 * @param body Body expression.
 * @return A @ref do_while_builder awaiting the condition expression.
 */
template<typename BodyExpression>
[[nodiscard]] constexpr auto do_(BodyExpression&& body) noexcept(
  std::is_nothrow_constructible_v<make_deferred_t<BodyExpression>, BodyExpression&&>)
{
  return do_while_builder<make_deferred_t<BodyExpression>>(std::forward<BodyExpression>(body));
}

} // namespace deferred

#endif
