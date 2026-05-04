// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_CONDITIONAL_HPP
#define DEFERRED_CONDITIONAL_HPP

#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

/**
 * @brief Deferred conditional that evaluates @p ThenExpression if
 * @p ConditionExpression evaluates to @c true.
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam ThenExpression Type of the expression to evaluate if the condition is @c true.
 */
template<Deferred ConditionExpression, Deferred ThenExpression>
class if_then_expression
{
public:
  using condition_expression_type = ConditionExpression;
  using then_expression_type      = ThenExpression;
  using then_result_type          = decltype(std::declval<ThenExpression>()());
  using result_type =
    std::conditional_t<std::is_void_v<then_result_type>, void, std::optional<then_result_type>>;
  using subexpression_types = std::tuple<ConditionExpression, ThenExpression>;

private:
  [[no_unique_address]] condition_expression_type m_condition;
  [[no_unique_address]] then_expression_type m_then;

public:
  /**
   * @brief Constructs an if_then_expression.
   * @tparam Condition Type of the condition expression.
   * @tparam ThenEx Type of the then expression.
   * @param condition Condition expression.
   * @param then_ Then expression.
   */
  template<typename Condition, typename ThenEx>
  constexpr explicit if_then_expression(Condition&& condition, ThenEx&& then_) :
    m_condition(std::forward<Condition>(condition)), m_then(std::forward<ThenEx>(then_))
  { }

  /**
   * @brief Evaluates the conditional expression.
   * @return Result of the conditional expression.
   */
  [[nodiscard]] constexpr result_type operator()() const
  {
    if (evaluate(m_condition))
    {
      if constexpr (std::is_void_v<then_result_type>)
      {
        evaluate(m_then);
      }
      else
      {
        return std::optional<then_result_type>{evaluate(m_then)};
      }
    }
    else
    {
      if constexpr (!std::is_void_v<then_result_type>)
      {
        return std::nullopt;
      }
    }
  }

  /// @copydoc if_then_expression::operator()() const
  [[nodiscard]] constexpr result_type operator()()
  {
    if (evaluate(m_condition))
    {
      if constexpr (std::is_void_v<then_result_type>)
      {
        evaluate(m_then);
      }
      else
      {
        return std::optional<then_result_type>{evaluate(m_then)};
      }
    }
    else
    {
      if constexpr (!std::is_void_v<then_result_type>)
      {
        return std::nullopt;
      }
    }
  }

  /**
   * @brief Visits the expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    std::forward<Visitor>(v)(m_condition, nesting + 1);
    std::forward<Visitor>(v)(m_then, nesting + 1);
  }
};

/**
 * @brief Deferred conditional that evaluates @p ThenExpression if
 * @p ConditionExpression evaluates to @c true, otherwise it evaluates
 * @p ElseExpression.
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam ThenExpression Type of the expression to evaluate if the condition is @c true.
 * @tparam ElseExpression Type of the expression to evaluate if the condition is @c false.
 */
template<Deferred ConditionExpression, Deferred ThenExpression, Deferred ElseExpression>
class if_then_else_expression
{
public:
  using condition_expression_type = ConditionExpression;
  using then_expression_type      = ThenExpression;
  using else_expression_type      = ElseExpression;
  using result_type               = std::common_type_t<decltype(std::declval<ThenExpression>()()),
                                                       decltype(std::declval<ElseExpression>()())>;
  using subexpression_types       = std::tuple<ConditionExpression, ThenExpression, ElseExpression>;

private:
  [[no_unique_address]] condition_expression_type m_condition;
  [[no_unique_address]] then_expression_type m_then;
  [[no_unique_address]] else_expression_type m_else;

public:
  /**
   * @brief Constructs an if_then_else_expression.
   * @tparam Condition Type of the condition expression.
   * @tparam ThenEx Type of the then expression.
   * @tparam ElseEx Type of the else expression.
   * @param condition Condition expression.
   * @param then_ Then expression.
   * @param else_ Else expression.
   */
  template<typename Condition, typename ThenEx, typename ElseEx>
  constexpr explicit if_then_else_expression(Condition&& condition,
                                             ThenEx&& then_,
                                             ElseEx&& else_) :
    m_condition(std::forward<Condition>(condition)), m_then(std::forward<ThenEx>(then_)),
    m_else(std::forward<ElseEx>(else_))
  { }

  /**
   * @brief Evaluates the conditional expression.
   * @return Result of the conditional expression.
   */
  [[nodiscard]] constexpr result_type operator()() const
  {
    if (evaluate(m_condition))
    {
      return evaluate(m_then);
    }
    else
    {
      return evaluate(m_else);
    }
  }

  /// @copydoc if_then_else_expression::operator()() const
  [[nodiscard]] constexpr result_type operator()()
  {
    if (evaluate(m_condition))
    {
      return evaluate(m_then);
    }
    else
    {
      return evaluate(m_else);
    }
  }

  /**
   * @brief Visits the expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    std::forward<Visitor>(v)(m_condition, nesting + 1);
    std::forward<Visitor>(v)(m_then, nesting + 1);
    std::forward<Visitor>(v)(m_else, nesting + 1);
  }
};

/**
 * @brief Creates a @c deferred conditional that evaluates @p then_ if @p condition
 * evaluates to @c true, otherwise it evaluates @p else_.
 *
 * The result type of <tt>if_then_else_expression(...)()</tt> is the
 * @c std::common_type of the result types of @p then_ and @p else_.
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam ThenExpression Type of the then expression.
 * @tparam ElseExpression Type of the else expression.
 * @param condition Condition expression.
 * @param then_ Then expression.
 * @param else_ Else expression.
 * @return An @ref if_then_else_expression that will perform the deferred evaluation.
 */
template<typename ConditionExpression, typename ThenExpression, typename ElseExpression>
[[nodiscard]] constexpr auto
if_then_else(ConditionExpression&& condition, ThenExpression&& then_, ElseExpression&& else_)
{
  using condition_expression = make_deferred_t<ConditionExpression>;
  using then_expression      = make_deferred_t<ThenExpression>;
  using else_expression      = make_deferred_t<ElseExpression>;
  return if_then_else_expression<condition_expression, then_expression, else_expression>(
    std::forward<ConditionExpression>(condition),
    std::forward<ThenExpression>(then_),
    std::forward<ElseExpression>(else_));
}

/**
 * @brief Creates a @c deferred conditional that evaluates @p then_ if @p condition
 * evaluates to @c true.
 *
 * The result type of <tt>if_then_expression(...)()</tt> is @c std::optional<T> if
 * @p then_ returns @c T (where @c T is not @c void), otherwise it is @c void.
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam ThenExpression Type of the then expression.
 * @param condition Condition expression.
 * @param then_ Then expression.
 * @return An @ref if_then_expression that will perform the deferred evaluation.
 */
template<typename ConditionExpression, typename ThenExpression>
[[nodiscard]] constexpr auto if_then(ConditionExpression&& condition, ThenExpression&& then_)
{
  using condition_expression = make_deferred_t<ConditionExpression>;
  using then_expression      = make_deferred_t<ThenExpression>;
  return if_then_expression<condition_expression, then_expression>(
    std::forward<ConditionExpression>(condition),
    std::forward<ThenExpression>(then_));
}

} // namespace deferred

#endif
