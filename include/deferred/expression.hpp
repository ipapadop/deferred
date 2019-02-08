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

#include "type_traits.hpp"

namespace deferred
{

/**
 * TODO
 */
template<typename Operator, typename... Expression>
class expression_
  : private Operator,
    private std::tuple<Expression...>
{
public:
  template<typename Op, typename... Ex>
  constexpr explicit expression_(Op&& op, Ex&&... ex)
    : Operator(std::forward<Op>(op)),
      std::tuple<Expression...>(std::forward<Ex>(ex)...)
  {}

private:
  template<std::size_t... I>
  constexpr decltype(auto) call(std::index_sequence<I...>) const
  {
    return Operator::operator()(std::get<I>(*this)()...);
  }

public:
  constexpr decltype(auto) operator()() const
  {
    return call(std::index_sequence_for<Expression...>{});
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/// Checks if @p T is an @ref expression_.
template<typename...>
struct is_expression
  : public std::false_type
{};

template<typename... T>
struct is_expression<expression_<T...>>
  : public std::true_type
{};

/// Alias for @c is_expression::type.
template<typename T>
using is_expression_t = typename is_expression<T>::type;

/// Alias for @c is_expression::value.
template<typename T>
inline constexpr bool is_expression_v = is_expression<T>::value;

// Transforms T into an expression_ if it is not a deferred data type.
template<typename T, typename... Args>
using make_expression_t =
  std::conditional_t<
    is_expression_v<std::decay_t<T>>,
    T,
    expression_<T, Args...>>;

template<typename... T>
struct is_deferred<expression_<T...>>
  : public std::true_type
{};

} // namespace deferred

#endif
