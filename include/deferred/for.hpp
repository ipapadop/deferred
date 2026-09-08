// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_FOR_HPP
#define DEFERRED_FOR_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/is_nothrow_visitable.hpp"
#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

/**
 * @brief Deferred for loop that evaluates @p InitExpression once per evaluation of the
 * loop, then evaluates @p BodyExpression followed by @p StepExpression while
 * @p ConditionExpression evaluates to @c true.
 *
 * @tparam InitExpression Type of the initialization expression.
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam StepExpression Type of the step expression.
 * @tparam BodyExpression Type of the body expression.
 */
template<Deferred InitExpression,
         Deferred ConditionExpression,
         Deferred StepExpression,
         Deferred BodyExpression>
class for_expression
{
public:
  using subexpression_types =
    std::tuple<InitExpression, ConditionExpression, StepExpression, BodyExpression>;

private:
  [[no_unique_address]] InitExpression m_init;
  [[no_unique_address]] ConditionExpression m_condition;
  [[no_unique_address]] StepExpression m_step;
  [[no_unique_address]] BodyExpression m_body;

public:
  /**
   * @brief Constructs a for_expression.
   * @tparam Init Type of the initialization expression.
   * @tparam Condition Type of the condition expression.
   * @tparam Step Type of the step expression.
   * @tparam Body Type of the body expression.
   * @param init Initialization expression.
   * @param condition Condition expression.
   * @param step Step expression.
   * @param body Body expression.
   */
  template<typename Init, typename Condition, typename Step, typename Body>
  constexpr explicit for_expression(Init&& init, Condition&& condition, Step&& step, Body&& body) //
    noexcept(std::is_nothrow_constructible_v<InitExpression, Init&&>
             && std::is_nothrow_constructible_v<ConditionExpression, Condition&&>
             && std::is_nothrow_constructible_v<StepExpression, Step&&>
             && std::is_nothrow_constructible_v<BodyExpression, Body&&>) :
    m_init(std::forward<Init>(init)), m_condition(std::forward<Condition>(condition)),
    m_step(std::forward<Step>(step)), m_body(std::forward<Body>(body))
  { }

  /// @brief Evaluates the for loop.
  constexpr void operator()() const
    noexcept(noexcept(evaluate(m_init)) && noexcept(static_cast<bool>(evaluate(m_condition)))
             && noexcept(evaluate(m_step)) && noexcept(evaluate(m_body)))
  {
    for (evaluate(m_init); evaluate(m_condition); evaluate(m_step))
    {
      evaluate(m_body);
    }
  }

  /// @copydoc for_expression::operator()() const
  constexpr void operator()() noexcept(noexcept(evaluate(m_init))
                                       && noexcept(static_cast<bool>(evaluate(m_condition)))
                                       && noexcept(evaluate(m_step)) && noexcept(evaluate(m_body)))
  {
    for (evaluate(m_init); evaluate(m_condition); evaluate(m_step))
    {
      evaluate(m_body);
    }
  }

  /**
   * @brief Visits the for expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, for_expression, subexpression_types>)
  {
    v(*this, nesting);
    m_init.visit(v, nesting + 1);
    m_condition.visit(v, nesting + 1);
    m_step.visit(v, nesting + 1);
    m_body.visit(v, nesting + 1);
  }
};

/**
 * @brief For loop waiting for its body expression.
 * @tparam InitExpression Type of the initialization expression.
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam StepExpression Type of the step expression.
 */
template<Deferred InitExpression, Deferred ConditionExpression, Deferred StepExpression>
class for_builder
{
  /** @brief The @ref for_expression this builder completes with a body of type @p Body. */
  template<typename Body>
  using expression_type =
    for_expression<InitExpression, ConditionExpression, StepExpression, make_deferred_t<Body>>;

  [[no_unique_address]] InitExpression m_init;
  [[no_unique_address]] ConditionExpression m_condition;
  [[no_unique_address]] StepExpression m_step;

public:
  /**
   * @brief Constructs a for_builder.
   * @tparam Init Type of the initialization expression.
   * @tparam Condition Type of the condition expression.
   * @tparam Step Type of the step expression.
   * @param init Initialization expression.
   * @param condition Condition expression.
   * @param step Step expression awaiting a body expression.
   */
  template<typename Init, typename Condition, typename Step>
  constexpr explicit for_builder(Init&& init, Condition&& condition, Step&& step) //
    noexcept(std::is_nothrow_constructible_v<InitExpression, Init&&>
             && std::is_nothrow_constructible_v<ConditionExpression, Condition&&>
             && std::is_nothrow_constructible_v<StepExpression, Step&&>) :
    m_init(std::forward<Init>(init)), m_condition(std::forward<Condition>(condition)),
    m_step(std::forward<Step>(step))
  { }

  /**
   * @brief Completes the loop with its body expression.
   * @tparam BodyExpression Type of the body expression.
   * @param body Body expression.
   * @return A @ref for_expression capturing the initialization, condition, step and body.
   */
  template<typename BodyExpression>
  [[nodiscard]] constexpr auto do_(BodyExpression&& body) && //
    noexcept(std::is_nothrow_constructible_v<expression_type<BodyExpression>,
                                             InitExpression&&,
                                             ConditionExpression&&,
                                             StepExpression&&,
                                             BodyExpression&&>)
  {
    return expression_type<BodyExpression>(std::forward<InitExpression>(m_init),
                                           std::forward<ConditionExpression>(m_condition),
                                           std::forward<StepExpression>(m_step),
                                           std::forward<BodyExpression>(body));
  }

  /// @copydoc do_
  template<typename BodyExpression>
  [[nodiscard]] constexpr auto do_(BodyExpression&& body) const& //
    noexcept(std::is_nothrow_constructible_v<expression_type<BodyExpression>,
                                             InitExpression const&,
                                             ConditionExpression const&,
                                             StepExpression const&,
                                             BodyExpression&&>)
  {
    return expression_type<BodyExpression>(m_init,
                                           m_condition,
                                           m_step,
                                           std::forward<BodyExpression>(body));
  }
};

/**
 * @brief Starts a @c deferred for loop that evaluates @p init once per evaluation of the
 * loop and then runs while @p condition is @c true, evaluating @p step after every
 * iteration.
 *
 * Example:
 * @code
 * auto ex = for_(init, condition, step).do_(body);
 * @endcode
 *
 * @tparam InitExpression Type of the initialization expression.
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam StepExpression Type of the step expression.
 * @param init Initialization expression.
 * @param condition Condition expression.
 * @param step Step expression.
 * @return A @ref for_builder awaiting the body expression.
 */
template<typename InitExpression, typename ConditionExpression, typename StepExpression>
[[nodiscard]] constexpr auto
for_(InitExpression&& init, ConditionExpression&& condition, StepExpression&& step) //
  noexcept(
    std::is_nothrow_constructible_v<make_deferred_t<InitExpression>, InitExpression&&>
    && std::is_nothrow_constructible_v<make_deferred_t<ConditionExpression>, ConditionExpression&&>
    && std::is_nothrow_constructible_v<make_deferred_t<StepExpression>, StepExpression&&>)
{
  return for_builder<make_deferred_t<InitExpression>,
                     make_deferred_t<ConditionExpression>,
                     make_deferred_t<StepExpression>>(std::forward<InitExpression>(init),
                                                      std::forward<ConditionExpression>(condition),
                                                      std::forward<StepExpression>(step));
}

} // namespace deferred

#endif
