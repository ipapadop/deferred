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

#include <ostream>
#include <tuple>
#include <type_traits>
#include <utility>

#include "constant.hpp"
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
  constexpr expression_(Op&& op, Ex&&... ex)
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

/**
 * Checks if @p T is an @ref expression_.
 */
template<typename... T>
struct is_expression
  : public std::false_type
{};

template<typename... U>
struct is_expression<expression_<U...>>
  : public std::true_type
{};

template<typename... T>
struct is_deferred<expression_<T...>>
  : public std::true_type
{};

/**
 * Transforms @p T into a @c deferred data type if it is not already.
 */
template<typename T>
using make_datatype_t =
  std::conditional_t<
    is_deferred_t<std::decay_t<T>>::value,
    T,
    constant_<T>>;

/**
 * TODO
 */
template<typename Operator, typename... Expression>
constexpr auto make_expression(Operator&& op, Expression&&... ex)
{
  using expression_type = expression_<Operator, make_datatype_t<Expression>...>;
  return expression_type(std::forward<Operator>(op), std::forward<Expression>(ex)...);
}

} // namespace deferred

#endif
