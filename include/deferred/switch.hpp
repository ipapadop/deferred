// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_SWITCH_HPP
#define DEFERRED_SWITCH_HPP

#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "evaluate.hpp"
#include "expression.hpp"
#include "type_traits/homogenized_type.hpp"

namespace deferred {

/**
 * @brief Switch default expression.
 * @tparam Expression Type of the underlying expression.
 */
template<Deferred Expression>
class default_expression
{
public:
  using subexpression_types = std::tuple<Expression>;
  using body_type           = Expression;

private:
  [[no_unique_address]] Expression m_expression;

public:
  /**
   * @brief Constructs a default_expression.
   * @tparam T Type of the arguments.
   * @param t Arguments to forward to the underlying expression.
   */
  template<typename T>
  constexpr explicit default_expression(T&& t) : m_expression(std::forward<T>(t))
  { }

  [[nodiscard]] constexpr decltype(auto) operator()() const
  {
    return evaluate(m_expression);
  }

  /// @copydoc default_expression::operator()() const
  [[nodiscard]] constexpr decltype(auto) operator()()
  {
    return evaluate(m_expression);
  }

  /**
   * @brief Visits the default expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v The visitor.
   * @param nesting The nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    std::forward<Visitor>(v)(m_expression, nesting + 1);
  }
};

/**
 * @brief Switch case expression.
 * @tparam LabelExpression Type of the label expression.
 * @tparam BodyExpression Type of the body expression.
 */
template<Deferred LabelExpression, Deferred BodyExpression>
class case_expression
{
public:
  using label_expression_type = LabelExpression;
  using body_expression_type  = BodyExpression;
  using subexpression_types   = std::tuple<LabelExpression, BodyExpression>;
  using body_type             = BodyExpression;

private:
  [[no_unique_address]] LabelExpression m_label;
  [[no_unique_address]] BodyExpression m_body;

public:
  /**
   * @brief Constructs a case_expression.
   * @tparam LabelEx Type of the label expression.
   * @tparam BodyEx Type of the body expression.
   * @param label The label expression.
   * @param body The body expression.
   */
  template<typename LabelEx, typename BodyEx>
  constexpr explicit case_expression(LabelEx&& label, BodyEx&& body) :
    m_label(std::forward<LabelEx>(label)), m_body(std::forward<BodyEx>(body))
  { }

  /// @brief Compares @p T with the label expression.
  template<typename T>
  [[nodiscard]] constexpr decltype(auto) compare(T&& t) const
  {
    return std::forward<T>(t) == evaluate(m_label);
  }

  /// @brief Returns the result of the body expression.
  [[nodiscard]] constexpr decltype(auto) operator()() const
  {
    return evaluate(m_body);
  }

  /// @copydoc case_expression::operator()() const
  [[nodiscard]] constexpr decltype(auto) operator()()
  {
    return evaluate(m_body);
  }

  /**
   * @brief Visits the case expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v The visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    std::forward<Visitor>(v)(m_label, nesting + 1);
    std::forward<Visitor>(v)(m_body, nesting + 1);
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

/**
 * @brief Maps the result of an evaluation to the target result type.
 * @tparam Result Target result type.
 * @tparam T Type of the evaluated expression.
 * @param t Evaluated expression.
 * @return Mapped result.
 */
template<typename Result, typename T>
constexpr decltype(auto) map_switch_result(T&& t)
{
  if constexpr (std::is_void_v<Result>)
  {
    static_cast<void>(t);
  }
  else if constexpr (std::is_void_v<T>)
  {
    static_cast<void>(t);
    return std::monostate{};
  }
  else
  {
    return std::forward<T>(t);
  }
}

} // namespace detail

/**
 * @brief Deferred switch
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam DefaultExpression Type of the default case expression.
 * @tparam CaseExpression Types of the case expressions.
 */
template<Deferred ConditionExpression, Deferred DefaultExpression, Deferred... CaseExpression>
class switch_expression
{
public:
  using condition_expression_type = ConditionExpression;
  using default_expression_type   = DefaultExpression;
  using case_expression_types     = std::tuple<CaseExpression...>;
  using subexpression_types = std::tuple<ConditionExpression, DefaultExpression, CaseExpression...>;

  /**
   * @brief Result type of the switch expression (common type or variant).
   */
  using result_type = homogenized_type_t<decltype(std::declval<typename DefaultExpression::body_type>()()),
                                         decltype(std::declval<typename CaseExpression::body_type>()())...>;

private:
  [[no_unique_address]] ConditionExpression m_condition;
  [[no_unique_address]] std::tuple<DefaultExpression, CaseExpression...> m_cases;

public:
  /**
   * @brief Constructs a switch_expression.
   * @tparam Condition Type of the condition expression.
   * @tparam Default Type of the default expression.
   * @tparam Case Types of the case expressions.
   * @param condition Condition expression.
   * @param df Default expression.
   * @param cs Case expressions.
   */
  template<typename Condition, typename Default, typename... Case>
  constexpr explicit switch_expression(Condition&& condition, Default&& df, Case&&... cs) :
    m_condition(std::forward<Condition>(condition)),
    m_cases(std::forward<Default>(df), std::forward<Case>(cs)...)
  { }

private:
  /**
   * @brief Traverses the cases until one matches.
   *
   * If none does, the default (@c std::tuple_element<0>) is returned.
   */
  template<std::size_t I, typename T>
  [[nodiscard]] constexpr result_type choose_case(T&& t) const
  {
    if constexpr (I < std::tuple_size<decltype(m_cases)>::value)
    {
      if (std::get<I>(m_cases).compare(std::forward<T>(t)))
      {
        return detail::map_switch_result<result_type>(std::get<I>(m_cases)());
      }

      return choose_case<I + 1>(std::forward<T>(t));
    }
    else
    {
      return detail::map_switch_result<result_type>(std::get<0>(m_cases)());
    }
  }

  /// @copydoc switch_expression::choose_case(T&&) const
  template<std::size_t I, typename T>
  [[nodiscard]] constexpr result_type choose_case(T&& t)
  {
    if constexpr (I < std::tuple_size<decltype(m_cases)>::value)
    {
      if (std::get<I>(m_cases).compare(std::forward<T>(t)))
      {
        return detail::map_switch_result<result_type>(std::get<I>(m_cases)());
      }

      return choose_case<I + 1>(std::forward<T>(t));
    }
    else
    {
      return detail::map_switch_result<result_type>(std::get<0>(m_cases)());
    }
  }

public:
  /**
   * @brief Evaluates the switch expression.
   * @return Result of the switch expression.
   */
  [[nodiscard]] constexpr result_type operator()() const
  {
    // start from second case, as first is the default
    return choose_case<1>(evaluate(m_condition));
  }

  /// @copydoc switch_expression::operator()() const
  [[nodiscard]] constexpr result_type operator()()
  {
    // start from second case, as first is the default
    return choose_case<1>(evaluate(m_condition));
  }

  /**
   * @brief Visits the switch expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v The visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    std::forward<Visitor>(v)(m_condition, nesting + 1);
    std::apply([&v, nesting](
                 auto const&... args) { (args.visit(std::forward<Visitor>(v), nesting + 1), ...); },
               m_cases);
  }
};

/**
 * @brief Creates a default case for use with @ref switch_().
 * @tparam Expression Type of the default expression.
 * @param ex Expression to use as a default case.
 * @return A @ref default_expression wrapping the given expression.
 */
template<typename Expression>
[[nodiscard]] constexpr auto default_(Expression&& ex)
{
  using expression = make_deferred_t<Expression>;
  return default_expression<expression>(std::forward<Expression>(ex));
}

/**
 * @brief Creates a case for use with @ref switch_().
 * @tparam LabelExpression Type of the label expression.
 * @tparam BodyExpression Type of the body expression.
 * @param label Label expression to compare against.
 * @param body Body expression to evaluate if matched.
 * @return A @ref case_expression wrapping the label and body.
 */
template<typename LabelExpression, typename BodyExpression>
[[nodiscard]] constexpr auto case_(LabelExpression&& label, BodyExpression&& body)
{
  using label_expression = make_deferred_t<LabelExpression>;
  using body_expression  = make_deferred_t<BodyExpression>;
  return case_expression<label_expression, body_expression>(std::forward<LabelExpression>(label),
                                                            std::forward<BodyExpression>(body));
}

/**
 * @brief Creates a new @ref switch_expression that checks @p condition against the
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
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam DefaultExpression Type of the default expression.
 * @tparam CaseExpressions Types of the case expressions.
 * @param condition Condition expression.
 * @param default_ Default case expression.
 * @param case_ Case expressions.
 * @return A tuple-like expression representing the switch construct.
 */
template<typename ConditionExpression, typename DefaultExpression, typename... CaseExpressions>
[[nodiscard]] constexpr auto
switch_(ConditionExpression&& condition, DefaultExpression&& default_, CaseExpressions&&... case_)
{
  static_assert(detail::is_valid_default<std::decay_t<DefaultExpression>>::value,
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
