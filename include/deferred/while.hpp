// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_WHILE_HPP
#define DEFERRED_WHILE_HPP

#include <tuple>
#include <utility>

#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

/**
 * @brief Deferred while loop that evaluates @p BodyExpression while
 * @p ConditionExpression evaluates to @c true.
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam BodyExpression Type of the body expression.
 */
template<Deferred ConditionExpression, Deferred BodyExpression>
class while_expression
{
public:
  using condition_expression_type = ConditionExpression;
  using body_expression_type      = BodyExpression;
  using subexpression_types       = std::tuple<ConditionExpression, BodyExpression>;

private:
  [[no_unique_address]] ConditionExpression m_condition;
  [[no_unique_address]] BodyExpression m_body;

public:
  /**
   * @brief Constructs a while_expression.
   * @tparam Condition Type of the condition expression.
   * @tparam Body Type of the body expression.
   * @param condition Condition expression.
   * @param body Body expression.
   */
  template<typename Condition, typename Body>
  constexpr explicit while_expression(Condition&& condition, Body&& body) :
    m_condition(std::forward<Condition>(condition)), m_body(std::forward<Body>(body))
  { }

  /// @brief Evaluates the while loop.
  constexpr void operator()() const&
  {
    while (evaluate(m_condition))
    {
      evaluate(m_body);
    }
  }

  /// @copydoc while_expression::operator()() const&
  constexpr void operator()() &
  {
    while (evaluate(m_condition))
    {
      evaluate(m_body);
    }
  }

  /// @copydoc while_expression::operator()() const&
  constexpr void operator()() &&
  {
    while (evaluate(m_condition))
    {
      evaluate(m_body);
    }
  }

  /**
   * @brief Visits the while expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    m_condition.visit(std::forward<Visitor>(v), nesting + 1);
    m_body.visit(std::forward<Visitor>(v), nesting + 1);
  }
};

/**
 * @brief Creates a @c deferred while that calls @p body while @p condition is @c true.
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam BodyExpression Type of the body expression.
 * @param condition Condition expression.
 * @param body Body expression.
 * @return A @ref while_expression capturing the condition and body.
 */
template<typename ConditionExpression, typename BodyExpression>
[[nodiscard]] constexpr auto while_(ConditionExpression&& condition, BodyExpression&& body)
{
  using condition_expression = make_deferred_t<ConditionExpression>;
  using body_expression      = make_deferred_t<BodyExpression>;
  return while_expression<condition_expression, body_expression>(
    std::forward<ConditionExpression>(condition),
    std::forward<BodyExpression>(body));
}

} // namespace deferred

#endif
