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

/// Switch case expression.
template<typename LabelExpression, typename BodyExpression>
class case_expression : private std::tuple<LabelExpression, BodyExpression>
{
public:
  using constant_expression =
    std::conjunction<is_constant_expression<LabelExpression>,
                     is_constant_expression<BodyExpression>>;

  using subexpression_types = std::tuple<LabelExpression, BodyExpression>;

  template<typename LabelEx, typename BodyEx>
  constexpr explicit case_expression(LabelEx&& label, BodyEx&& body) :
    std::tuple<LabelExpression, BodyExpression>(std::forward<LabelEx>(label),
                                                std::forward<BodyEx>(body))
  {}

  /// Compares @p T with the label expression.
  template<typename T>
  constexpr bool compare(T&& t) const
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

/**
 * Deferred switch
 */
template<typename SwitchExpression, typename... CaseExpression>
class switch_expression :
  private std::tuple<SwitchExpression, CaseExpression...>
{
public:
  using constant_expression =
    std::conjunction<is_constant_expression<SwitchExpression>,
                     is_constant_expression<CaseExpression>...>;

  using subexpression_types = std::tuple<SwitchExpression, CaseExpression...>;

  template<typename SwitchEx, typename... CaseEx>
  constexpr explicit switch_expression(SwitchEx&& sw, CaseEx&&... cs) :
    std::tuple<SwitchExpression, CaseExpression...>(std::forward<SwitchEx>(sw),
                                                    std::forward<CaseEx>(cs)...)
  {}

  constexpr decltype(auto) operator()() const
  {
    if (std::get<0>(static_cast<subexpression_types const&>(*this))())
    {
      return std::get<1>(static_cast<subexpression_types const&>(*this))();
    }
    else
    {
      return std::get<2>(static_cast<subexpression_types const&>(*this))();
    }
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

template<typename LabelExpression, typename BodyExpression>
auto case_(LabelExpression&& label, BodyExpression&& body)
{
  using label_expression = make_expression_t<LabelExpression>;
  using body_expression  = make_expression_t<BodyExpression>;
  return case_expression<label_expression, body_expression>(
    std::forward<LabelExpression>(label), std::forward<BodyExpression>(body));
}

template<typename Expression>
auto default_(Expression&& ex)
{
  using expression = make_expression_t<Expression>;
  return expression(std::forward<Expression>(ex));
}

template<typename SwitchExpression, typename... Expressions>
auto switch_(SwitchExpression&& sw, Expressions&&... ex)
{
  using switch_expression = make_expression_t<SwitchExpression>;
  return switch_expression(std::forward<SwitchExpression>(sw));
}

} // namespace deferred

#endif
