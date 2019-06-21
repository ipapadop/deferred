/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_SWITCH_HPP
#define DEFERRED_SWITCH_HPP

#include <type_traits>
#include <utility>

#include "expression.hpp"
#include "type_traits/is_constant_expression.hpp"

namespace deferred {

/// Switch default expression.
template<typename Expression>
class default_expression : private Expression
{
public:
  using Expression::Expression;
  using Expression::operator();
  using Expression::visit;
};

/// Switch case expression.
template<typename LabelExpression, typename BodyExpression>
class case_expression : private std::tuple<LabelExpression, BodyExpression>
{
private:
  using subexpression_types = std::tuple<LabelExpression, BodyExpression>;

public:
  using constant_expression =
    std::conjunction<is_constant_expression<LabelExpression>,
                     is_constant_expression<BodyExpression>>;

  template<typename LabelEx, typename BodyEx>
  constexpr explicit case_expression(LabelEx&& label, BodyEx&& body) :
    std::tuple<LabelExpression, BodyExpression>(std::forward<LabelEx>(label),
                                                std::forward<BodyEx>(body))
  {}

  /// Compares @p T with the label expression.
  template<typename T>
  constexpr decltype(auto) compare(T&& t) const
  {
    auto& ex = std::get<0>(static_cast<subexpression_types const&>(*this));
    return std::forward<T>(t) == ex();
  }

  /// Returns the result of the body expression.
  constexpr decltype(auto) operator()() const
  {
    return std::get<1>(static_cast<subexpression_types const&>(*this))();
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

namespace detail {

template<typename>
struct is_valid_default : public std::false_type
{};

template<typename T>
struct is_valid_default<default_expression<T>> : public std::true_type
{};

template<typename>
struct is_valid_case : public std::false_type
{};

template<typename T, typename U>
struct is_valid_case<case_expression<T, U>> : public std::true_type
{};

} // namespace detail

/**
 * Deferred switch
 */
template<typename ConditionExpression,
         typename DefaultExpression,
         typename... CaseExpression>
class switch_expression :
  private std::tuple<ConditionExpression, DefaultExpression, CaseExpression...>
{
private:
  using subexpression_types =
    std::tuple<ConditionExpression, DefaultExpression, CaseExpression...>;

public:
  using constant_expression =
    std::conjunction<is_constant_expression<ConditionExpression>,
                     is_constant_expression<DefaultExpression>,
                     is_constant_expression<CaseExpression>...>;

  using result_type =
    std::common_type_t<decltype(std::declval<DefaultExpression>()()),
                       decltype(std::declval<CaseExpression>()())...>;

  template<typename Condition, typename Default, typename... Case>
  constexpr explicit switch_expression(Condition&& condition,
                                       Default&& df,
                                       Case&&... cs) :
    std::tuple<ConditionExpression, Default, CaseExpression...>(
      std::forward<Condition>(condition),
      std::forward<Default>(df),
      std::forward<Case>(cs)...)
  {}

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
      auto& case_ex =
        std::get<I>(static_cast<subexpression_types const&>(*this));
      if (case_ex.compare(std::forward<T>(t)))
      {
        return case_ex();
      }

      return choose_case<I + 1>(std::forward<T>(t));
    }
    else
    {
      auto& default_ex = std::get<DefaultExpression>(
        static_cast<subexpression_types const&>(*this));
      return default_ex();
    }
  }

  /// @copydoc choose_case(T&&) const
  template<std::size_t I, typename T>
  constexpr result_type choose_case(T&& t)
  {
    if constexpr (I < std::tuple_size<subexpression_types>::value)
    {
      auto& case_ex = std::get<I>(static_cast<subexpression_types&>(*this));
      if (case_ex.compare(std::forward<T>(t)))
      {
        return case_ex();
      }

      return choose_case<I + 1>(std::forward<T>(t));
    }
    else
    {
      auto& default_ex =
        std::get<DefaultExpression>(static_cast<subexpression_types&>(*this));
      return default_ex();
    }
  }

public:
  constexpr result_type operator()() const
  {
    // start from second case, as first is the default
    return choose_case<2>(std::get<ConditionExpression>(
      static_cast<subexpression_types const&>(*this))());
  }

  constexpr result_type operator()()
  {
    // start from second case, as first is the default
    return choose_case<2>(std::get<ConditionExpression>(
      static_cast<subexpression_types&>(*this))());
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/// Creates a default case for use with @ref switch_().
template<typename Expression>
auto default_(Expression&& ex)
{
  using expression = make_deferred_t<Expression>;
  return default_expression<expression>(std::forward<Expression>(ex));
}

/// Creates a case for use with @ref switch_().
template<typename LabelExpression, typename BodyExpression>
auto case_(LabelExpression&& label, BodyExpression&& body)
{
  using label_expression = make_deferred_t<LabelExpression>;
  using body_expression  = make_deferred_t<BodyExpression>;
  return case_expression<label_expression, body_expression>(
    std::forward<LabelExpression>(label), std::forward<BodyExpression>(body));
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
 */
template<typename ConditionExpression,
         typename DefaultExpression,
         typename... CaseExpressions>
constexpr auto switch_(ConditionExpression&& condition,
                       DefaultExpression&& default_,
                       CaseExpressions&&... case_)
{
  static_assert(
    std::conjunction_v<detail::is_valid_default<std::decay_t<DefaultExpression>>>,
    "Default case is not a valid deferred case expression");
  static_assert(
    std::conjunction_v<detail::is_valid_case<std::decay_t<CaseExpressions>>...>,
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
