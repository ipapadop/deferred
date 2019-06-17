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

#include "constant.hpp"
#include "expression.hpp"
#include "type_traits/is_constant_expression.hpp"
#include "type_traits/is_deferred.hpp"

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
    return std::forward<T>(t)
           == std::get<0>(static_cast<subexpression_types const&>(*this))();
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
template<typename SwitchExpression,
         typename DefaultExpression,
         typename... CaseExpression>
class switch_expression :
  private std::tuple<SwitchExpression, DefaultExpression, CaseExpression...>
{
private:
  using subexpression_types =
    std::tuple<SwitchExpression, DefaultExpression, CaseExpression...>;

public:
  using constant_expression =
    std::conjunction<is_constant_expression<SwitchExpression>,
                     is_constant_expression<DefaultExpression>,
                     is_constant_expression<CaseExpression>...>;

  using result_type =
    std::common_type_t<decltype(std::declval<CaseExpression>()())...>;

  template<typename SwitchEx, typename DefaultEx, typename... CaseEx>
  constexpr explicit switch_expression(SwitchEx&& sw,
                                       DefaultEx&& df,
                                       CaseEx&&... cs) :
    std::tuple<SwitchExpression, DefaultEx, CaseExpression...>(
      std::forward<SwitchEx>(sw),
      std::forward<DefaultEx>(df),
      std::forward<CaseEx>(cs)...)
  {}

private:
  /**
   * Traverses the cases until one matches.
   *
   * If none does, the default (@c 0) is returned.
   */
  template<std::size_t I, typename T>
  constexpr result_type choose_case(T&& t) const
  {
    if constexpr (I == std::tuple_size<subexpression_types>::value)
    {
      // return default case
      return std::get<1>(static_cast<subexpression_types const&>(*this))();
    }
    else
    {
      if (std::get<I>(static_cast<subexpression_types const&>(*this))
            .compare(std::forward<T>(t)))
      {
        return std::get<I>(static_cast<subexpression_types const&>(*this))();
      }

      return choose_case<I + 1>(std::forward<T>(t));
    }
  }

public:
  constexpr result_type operator()() const
  {
    // start from second case, as first is the default
    return choose_case<2>(
      std::get<0>(static_cast<subexpression_types const&>(*this))());
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

template<typename Expression>
auto default_(Expression&& ex)
{
  using expression = make_expression_t<Expression>;
  return default_expression<expression>(std::forward<Expression>(ex));
}

template<typename LabelExpression, typename BodyExpression>
auto case_(LabelExpression&& label, BodyExpression&& body)
{
  using label_expression = make_expression_t<LabelExpression>;
  using body_expression  = make_expression_t<BodyExpression>;
  return case_expression<label_expression, body_expression>(
    std::forward<LabelExpression>(label), std::forward<BodyExpression>(body));
}

template<typename SwitchExpression,
         typename DefaultExpression,
         typename... CaseExpressions>
auto switch_(SwitchExpression&& sw,
             DefaultExpression&& df,
             CaseExpressions&&... ex)
{
  static_assert(
    std::conjunction_v<detail::is_valid_default<std::decay_t<DefaultExpression>>>,
    "Default case is not a valid deferred case expression");
  static_assert(
    std::conjunction_v<detail::is_valid_case<std::decay_t<CaseExpressions>>...>,
    "One or more cases are not valid deferred case expressions");
  return switch_expression<make_expression_t<SwitchExpression>,
                           std::decay_t<DefaultExpression>,
                           std::decay_t<CaseExpressions>...>(
    std::forward<SwitchExpression>(sw),
    std::forward<DefaultExpression>(df),
    std::forward<CaseExpressions>(ex)...);
}

} // namespace deferred

#endif
