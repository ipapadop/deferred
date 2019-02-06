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
template<typename...>
class expression_;

template<typename Operator, typename Expression>
class expression_<Operator, Expression>
  : private Operator,
    private std::tuple<Expression>
{
public:
  template<typename Op, typename Ex>
  constexpr expression_(Op&& op, Ex&& ex)
    : Operator(std::forward<Op>(op)),
      std::tuple<Expression>(std::forward<Ex>(ex))
  {}

  constexpr auto operator()() const
  {
    return Operator::operator()(std::get<0>(*this)());
  }

  template<typename Visitor>
  constexpr auto visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};


template<typename Operator, typename Expression1, typename Expression2>
class expression_<Operator, Expression1, Expression2>
  : private Operator,
    private std::tuple<Expression1, Expression2>
{
public:
  template<typename Op, typename Ex1, typename Ex2>
  constexpr expression_(Op&& op, Ex1&& ex1, Ex2&& ex2)
    : Operator(std::forward<Op>(op)),
      std::tuple<Expression1, Expression2>(
        std::forward<Ex1>(ex1),
        std::forward<Ex2>(ex2))
  {}

  constexpr auto operator()() const
  {
    return Operator::operator()(std::get<0>(*this)(), std::get<1>(*this)());
  }

  template<typename Visitor>
  constexpr auto visit(Visitor&& v) const
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

/**
 * Checks if @p T is a @ref constant_, @ref expression_ or @ref variable_.
 */
template<typename T>
struct is_deferred_datatype
  : public disjunction<
      is_constant<T>,
      is_expression<T>,
      is_variable<T>>
{};

/**
 * Alias for @c is_deferred_datatype::type.
 */
template<typename T>
using is_deferred_datatype_t = typename is_deferred_datatype<T>::type;

/**
 * Checks if any of the types @p T is a @c deferred data type.
 */
template<typename... T>
using any_deferred_datatypes_t = disjunction<is_deferred_datatype_t<std::decay_t<T>>...>;

/**
 * Transforms @p T into a @c deferred data type if it is not already.
 */
template<typename T>
using make_datatype_t =
  std::conditional_t<
    is_deferred_datatype_t<std::decay_t<T>>::value,
    T,
    constant_<T>>;

template<typename Operator, typename Expression>
auto make_expression(Operator&& op, Expression&& ex)
{
  using T = make_datatype_t<Expression>;

  return expression_<Operator, T>{std::forward<Operator>(op),
                                  std::forward<Expression>(ex)};
}

/**
 * TODO
 */
template<typename Operator, typename Expression1, typename Expression2>
auto make_expression(Operator&& op, Expression1&& ex1, Expression2&& ex2)
{
  using T = make_datatype_t<Expression1>;
  using U = make_datatype_t<Expression2>;

  return expression_<Operator, T, U>{std::forward<Operator>(op),
                                     std::forward<Expression1>(ex1),
                                     std::forward<Expression2>(ex2)};
}

} // namespace deferred

#endif
