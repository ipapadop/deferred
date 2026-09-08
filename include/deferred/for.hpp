// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_FOR_HPP
#define DEFERRED_FOR_HPP

#include <type_traits>
#include <utility>

#include "expression.hpp"
#include "seq.hpp"
#include "while.hpp"

namespace deferred {

/**
 * @brief Deferred for loop.
 *
 * A @c for loop is defined as its initialization followed by a @c while loop whose body
 * is the loop body followed by the step, and that is exactly how this is built: it holds
 * <tt>seq(init, while_(condition).do_(seq(body, step)))</tt> and runs it. The composition
 * is an implementation detail — a @c for loop is a construct of its own, so it gets a node
 * of its own, and a visitor sees a @ref for_expression rather than the parts.
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
  /// @brief The body of the underlying @c while loop: the loop body, then the step.
  using iteration_type = seq_expression<BodyExpression, StepExpression>;

  /// @brief The underlying @c while loop.
  using loop_type = while_expression<ConditionExpression, iteration_type>;

public:
  /// @brief The composition this loop is defined as.
  using composed_type = seq_expression<InitExpression, loop_type>;

  using subexpression_types = std::tuple<composed_type>;

private:
  [[no_unique_address]] composed_type m_composed;

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
    noexcept(std::is_nothrow_constructible_v<iteration_type, Body&&, Step&&>
             && std::is_nothrow_constructible_v<loop_type, Condition&&, iteration_type&&>
             && std::is_nothrow_constructible_v<composed_type, Init&&, loop_type&&>) :
    m_composed(std::forward<Init>(init),
               loop_type(std::forward<Condition>(condition),
                         iteration_type(std::forward<Body>(body), std::forward<Step>(step))))
  { }

  /// @brief Evaluates the for loop.
  constexpr void operator()() const noexcept(noexcept(m_composed()))
  {
    m_composed();
  }

  /// @copydoc for_expression::operator()() const
  constexpr void operator()() noexcept(noexcept(m_composed()))
  {
    m_composed();
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
    m_composed.visit(v, nesting + 1);
  }
};

/**
 * @brief For loop waiting for its body expression.
 *
 * @tparam InitExpression Type of the initialization expression.
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam StepExpression Type of the step expression.
 */
template<Deferred InitExpression, Deferred ConditionExpression, Deferred StepExpression>
class for_builder
{
  /**
   * @brief The loop this builder completes with a body of type @p Body.
   * @tparam Body Type of the body expression.
   */
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
  constexpr explicit for_builder(Init&& init, Condition&& condition, Step&& step) noexcept(
    std::is_nothrow_constructible_v<InitExpression, Init&&>
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
 * @warning A clause that assigns must assign a @c deferred expression:
 * <tt>for_(i = constant(0), ...)</tt>, not <tt>for_(i = 0, ...)</tt>. The latter
 * assigns while the loop is being built and leaves a clause that only reads @c i,
 * so the loop runs no iterations from its second evaluation onwards.
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
for_(InitExpression&& init, ConditionExpression&& condition, StepExpression&& step) noexcept(
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
