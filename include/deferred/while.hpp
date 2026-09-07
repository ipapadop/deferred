// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_WHILE_HPP
#define DEFERRED_WHILE_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/is_nothrow_visitable.hpp"
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
  constexpr explicit while_expression(Condition&& condition, Body&& body) noexcept(
    std::is_nothrow_constructible_v<ConditionExpression, Condition&&>
    && std::is_nothrow_constructible_v<BodyExpression, Body&&>) :
    m_condition(std::forward<Condition>(condition)), m_body(std::forward<Body>(body))
  { }

  /// @brief Evaluates the while loop.
  constexpr void operator()() const
    noexcept(noexcept(static_cast<bool>(evaluate(m_condition))) && noexcept(evaluate(m_body)))
  {
    while (evaluate(m_condition))
    {
      evaluate(m_body);
    }
  }

  /// @copydoc while_expression::operator()() const
  constexpr void operator()() noexcept(noexcept(static_cast<bool>(evaluate(m_condition)))
                                       && noexcept(evaluate(m_body)))
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
    noexcept(detail::is_nothrow_visitable_v<Visitor, while_expression, subexpression_types>)
  {
    v(*this, nesting);
    m_condition.visit(v, nesting + 1);
    m_body.visit(v, nesting + 1);
  }
};

/**
 * @brief While loop waiting for its body expression.
 * @tparam ConditionExpression Type of the condition expression.
 */
template<Deferred ConditionExpression>
class while_builder
{
  [[no_unique_address]] ConditionExpression m_condition;

public:
  /**
   * @brief Constructs a while_builder.
   * @tparam Condition Type of the condition expression.
   * @param condition Condition expression awaiting a body expression.
   */
  template<typename Condition>
    requires(!std::is_same_v<std::remove_cvref_t<Condition>, while_builder>)
  constexpr explicit while_builder(Condition&& condition) noexcept(
    std::is_nothrow_constructible_v<ConditionExpression, Condition&&>) :
    m_condition(std::forward<Condition>(condition))
  { }

  /**
   * @brief Completes the loop with its body expression.
   * @tparam BodyExpression Type of the body expression.
   * @param body Body expression.
   * @return A @ref while_expression capturing the condition and body.
   */
  template<typename BodyExpression>
  [[nodiscard]] constexpr auto do_(BodyExpression&& body) &&
  {
    using body_expression = make_deferred_t<BodyExpression>;
    return while_expression<ConditionExpression, body_expression>(
      std::forward<ConditionExpression>(m_condition),
      std::forward<BodyExpression>(body));
  }

  /// @copydoc do_
  template<typename BodyExpression>
  [[nodiscard]] constexpr auto do_(BodyExpression&& body) const&
  {
    using body_expression = make_deferred_t<BodyExpression>;
    return while_expression<ConditionExpression, body_expression>(
      m_condition,
      std::forward<BodyExpression>(body));
  }
};

/**
 * @brief Starts a @c deferred while loop that runs while @p condition is @c true.
 *
 * Example:
 * @code
 * auto ex = while_(cond).do_(body);
 * @endcode
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @param condition Condition expression.
 * @return A @ref while_builder awaiting the body expression.
 */
template<typename ConditionExpression>
[[nodiscard]] constexpr auto while_(ConditionExpression&& condition) noexcept(
  std::is_nothrow_constructible_v<make_deferred_t<ConditionExpression>, ConditionExpression&&>)
{
  return while_builder<make_deferred_t<ConditionExpression>>(
    std::forward<ConditionExpression>(condition));
}

} // namespace deferred

#endif
