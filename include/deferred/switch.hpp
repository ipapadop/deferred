/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_SWITCH_HPP
#define DEFERRED_SWITCH_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

/**
 * @brief Switch default expression.
 * @tparam Expression The type of the underlying expression.
 */
template<typename Expression>
class default_expression
{
  [[no_unique_address]] Expression m_expression;

public:
  using subexpression_types = std::tuple<Expression>;

  /**
   * @brief Constructs a default_expression.
   * @tparam T The types of the arguments.
   * @param t The arguments to forward to the underlying expression.
   */
  template<typename... T>
  constexpr explicit default_expression(T&&... t) : m_expression(std::forward<T>(t)...)
  { }

  constexpr decltype(auto) operator()() const
  {
    return m_expression();
  }

  /**
   * @brief Visits the default expression with a visitor.
   * @tparam Visitor The type of the visitor.
   * @param v The visitor.
   * @param nesting The nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    m_expression.visit(std::forward<Visitor>(v), nesting + 1);
  }
};

/**
 * @brief Switch case expression.
 * @tparam LabelExpression The type of the label expression.
 * @tparam BodyExpression The type of the body expression.
 */
template<typename LabelExpression, typename BodyExpression>
class case_expression
{
  [[no_unique_address]] std::tuple<LabelExpression, BodyExpression> m_expressions;

public:
  using label_expression_type = LabelExpression;
  using body_expression_type  = BodyExpression;
  using subexpression_types   = std::tuple<LabelExpression, BodyExpression>;

  /**
   * @brief Constructs a case_expression.
   * @tparam LabelEx The type of the label expression.
   * @tparam BodyEx The type of the body expression.
   * @param label The label expression.
   * @param body The body expression.
   */
  template<typename LabelEx, typename BodyEx>
  constexpr explicit case_expression(LabelEx&& label, BodyEx&& body) :
    m_expressions(std::forward<LabelEx>(label), std::forward<BodyEx>(body))
  { }

  /// Compares @p T with the label expression.
  template<typename T>
  constexpr decltype(auto) compare(T&& t) const
  {
    return std::forward<T>(t) == evaluate(std::get<0>(m_expressions));
  }

  /// Returns the result of the body expression.
  constexpr decltype(auto) operator()() const
  {
    return evaluate(std::get<1>(m_expressions));
  }

  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    std::apply([&v, nesting](
                 auto const&... args) { (args.visit(std::forward<Visitor>(v), nesting + 1), ...); },
               m_expressions);
  }
};

namespace detail {

template<typename>
struct is_valid_default : public std::false_type
{ };

template<typename T>
struct is_valid_default<default_expression<T>> : public std::true_type
{ };

template<typename>
struct is_valid_case : public std::false_type
{ };

template<typename T, typename U>
struct is_valid_case<case_expression<T, U>> : public std::true_type
{ };

} // namespace detail

/**
 * Deferred switch
 *
 * @tparam ConditionExpression The type of the condition expression.
 * @tparam DefaultExpression The type of the default case expression.
 * @tparam CaseExpression The types of the case expressions.
 */
template<typename ConditionExpression, typename DefaultExpression, typename... CaseExpression>
class switch_expression
{
  [[no_unique_address]] std::tuple<ConditionExpression, DefaultExpression, CaseExpression...>
    m_expressions;

public:
  using condition_expression_type = ConditionExpression;
  using default_expression_type   = DefaultExpression;
  using case_expression_types     = std::tuple<CaseExpression...>;
  using subexpression_types = std::tuple<ConditionExpression, DefaultExpression, CaseExpression...>;

  using result_type = std::common_type_t<decltype(std::declval<DefaultExpression>()()),
                                         decltype(std::declval<CaseExpression>()())...>;

  /**
   * @brief Constructs a switch_expression.
   * @tparam Condition The type of the condition.
   * @tparam Default The type of the default expression.
   * @tparam Case The types of the case expressions.
   * @param condition The condition expression.
   * @param df The default expression.
   * @param cs The case expressions.
   */
  template<typename Condition, typename Default, typename... Case>
  constexpr explicit switch_expression(Condition&& condition, Default&& df, Case&&... cs) :
    m_expressions(std::forward<Condition>(condition),
                  std::forward<Default>(df),
                  std::forward<Case>(cs)...)
  { }

private:
  /**
   * Traverses the cases until one matches.
   *
   * If none does, the default (@c std::tuple_element<1>) is returned.
   */
  template<std::size_t I, typename T>
  constexpr result_type choose_case(T&& t) const
  {
    if constexpr (I < std::tuple_size<subexpression_types>::value)
    {
      if (std::get<I>(m_expressions).compare(std::forward<T>(t)))
      {
        return std::get<I>(m_expressions)();
      }

      return choose_case<I + 1>(std::forward<T>(t));
    }
    else
    {
      return std::get<DefaultExpression>(m_expressions)();
    }
  }

  /// @copydoc choose_case(T&&) const
  template<std::size_t I, typename T>
  constexpr result_type choose_case(T&& t)
  {
    if constexpr (I < std::tuple_size<subexpression_types>::value)
    {
      if (std::get<I>(m_expressions).compare(std::forward<T>(t)))
      {
        return std::get<I>(m_expressions)();
      }

      return choose_case<I + 1>(std::forward<T>(t));
    }
    else
    {
      return std::get<DefaultExpression>(m_expressions)();
    }
  }

public:
  constexpr result_type operator()() const
  {
    // start from second case, as first is the default
    return choose_case<2>(evaluate(std::get<ConditionExpression>(m_expressions)));
  }

  constexpr result_type operator()()
  {
    // start from second case, as first is the default
    return choose_case<2>(evaluate(std::get<ConditionExpression>(m_expressions)));
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/**
 * @brief Creates a default case for use with @ref switch_().
 * @tparam Expression The type of the default expression.
 * @param ex The expression to use as a default case.
 * @return A \ref default_expression wrapping the given expression.
 */
template<typename Expression>
auto default_(Expression&& ex)
{
  using expression = make_deferred_t<Expression>;
  return default_expression<expression>(std::forward<Expression>(ex));
}

/**
 * @brief Creates a case for use with @ref switch_().
 * @tparam LabelExpression The type of the label expression.
 * @tparam BodyExpression The type of the body expression.
 * @param label The label expression to compare against.
 * @param body The body expression to evaluate if matched.
 * @return A \ref case_expression wrapping the label and body.
 */
template<typename LabelExpression, typename BodyExpression>
auto case_(LabelExpression&& label, BodyExpression&& body)
{
  using label_expression = make_deferred_t<LabelExpression>;
  using body_expression  = make_deferred_t<BodyExpression>;
  return case_expression<label_expression, body_expression>(std::forward<LabelExpression>(label),
                                                            std::forward<BodyExpression>(body));
}

/**
 * Creates a new @ref switch_expression that checks @p condition against the
 * list of cases @p case_.
 *
 * If none of @p case_ matches, it returns the result of @p default_.
 *
 * Example:
 * @code
 * auto var = variable<int>();
 * auto ex = switch_(var,
 *                   default_("unknown"),
 *                   case_(10,
 *                         [] { return "10"; }),
 *                   case_([] { return foo(); },
 *                         [] { return "result of foo"; }));
 * @endcode
 *
 * @tparam ConditionExpression The type of the condition expression.
 * @tparam DefaultExpression The type of the default expression.
 * @tparam CaseExpressions The types of the case expressions.
 * @param condition The condition expression.
 * @param default_ The default case expression.
 * @param case_ The case expressions.
 * @return A tuple-like expression representing the switch construct.
 */
template<typename ConditionExpression, typename DefaultExpression, typename... CaseExpressions>
constexpr auto
switch_(ConditionExpression&& condition, DefaultExpression&& default_, CaseExpressions&&... case_)
{
  static_assert(std::conjunction_v<detail::is_valid_default<std::decay_t<DefaultExpression>>>,
                "Default case is not a valid deferred case expression");
  static_assert(std::conjunction_v<detail::is_valid_case<std::decay_t<CaseExpressions>>...>,
                "One or more cases are not valid deferred case expressions");

  using condition_expression = make_deferred_t<ConditionExpression>;
  return switch_expression<condition_expression,
                           std::decay_t<DefaultExpression>,
                           std::decay_t<CaseExpressions>...>(
    std::forward<ConditionExpression>(condition),
    std::forward<DefaultExpression>(default_),
    std::forward<CaseExpressions>(case_)...);
}

} // namespace deferred

#endif
