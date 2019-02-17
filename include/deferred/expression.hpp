/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_EXPRESSION_HPP
#define DEFERRED_EXPRESSION_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "constant.hpp"
#include "type_traits/is_constant_expression.hpp"
#include "type_traits/is_deferred.hpp"
#include "type_traits/is_expression.hpp"
#include "type_traits/make_function_object.hpp"

namespace deferred {

/**
 * Deferred expression that is composed of an operator @p Operator applied to
 * subexpressions @p Expressions....
 */
template<typename Operator, typename... Expressions>
class expression_ : private Operator, private std::tuple<Expressions...>
{
public:
  using is_constant_expression =
    std::conjunction<is_constant_expression<Expressions>...>;

  using operator_type       = Operator;
  using subexpression_types = std::tuple<Expressions...>;

  template<typename Op, typename... Ex>
  constexpr explicit expression_(Op&& op, Ex&&... ex) :
    Operator(std::forward<Op>(op)), std::tuple<Expressions...>(
                                      std::forward<Ex>(ex)...)
  {}

private:
  template<std::size_t... I>
  constexpr decltype(auto) call(std::index_sequence<I...>) const
  {
    return Operator::operator()(std::get<I> (*this)()...);
  }

public:
  constexpr decltype(auto) operator()() const
  {
    return call(std::index_sequence_for<Expressions...>{});
  }

  operator_type const& operator_() const noexcept
  {
    return static_cast<Operator const&>(*this);
  }

  subexpression_types const& subexpressions() const noexcept
  {
    return static_cast<subexpression_types const&>(*this);
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

namespace detail {

// Transforms T into a constant_ if it is not a deferred data type.
template<typename T>
using make_expression_arg_t =
  std::conditional_t<is_deferred_v<T>, T, constant_<T>>;

} // namespace detail

/// Transforms @p T into an @ref expression_ if it is not a deferred data type.
template<typename T, typename... Args>
using make_expression_t =
  std::conditional_t<is_expression_v<T>,
                     T,
                     expression_<make_function_object_t<T>,
                                 detail::make_expression_arg_t<Args>...>>;

} // namespace deferred

#endif
