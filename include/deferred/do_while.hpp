// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DO_WHILE_HPP
#define DEFERRED_DO_WHILE_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/is_nothrow_visitable.hpp"
#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

/**
 * @brief Deferred do-while loop that evaluates @p BodyExpression once and then again
 * while @p ConditionExpression evaluates to @c true.
 *
 * @tparam BodyExpression Type of the body expression.
 * @tparam ConditionExpression Type of the condition expression.
 */
template<Deferred BodyExpression, Deferred ConditionExpression>
class do_while_expression
{
public:
  using subexpression_types = std::tuple<BodyExpression, ConditionExpression>;

private:
  [[no_unique_address]] BodyExpression m_body;
  [[no_unique_address]] ConditionExpression m_condition;

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
  constexpr void operator()() const
    noexcept(noexcept(evaluate(m_body)) && noexcept(static_cast<bool>(evaluate(m_condition))))
  {
    do
    {
      evaluate(m_body);
    }
    while (evaluate(m_condition));
  }

  /// @copydoc do_while_expression::operator()() const
  constexpr void operator()() noexcept(noexcept(evaluate(m_body))
                                       && noexcept(static_cast<bool>(evaluate(m_condition))))
  {
    do
    {
      evaluate(m_body);
    }
    while (evaluate(m_condition));
  }

  /**
   * @brief Visits the do-while expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
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
template<Deferred BodyExpression>
class do_while_builder
{
  [[no_unique_address]] BodyExpression m_body;

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
  template<typename ConditionExpression>
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
  template<typename ConditionExpression>
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
