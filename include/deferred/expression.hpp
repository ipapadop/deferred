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

#include "apply.hpp"
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
  using constant_expression =
    std::conjunction<is_constant_expression<Expressions>...>;

  using operator_type       = Operator;
  using subexpression_types = std::tuple<Expressions...>;

  template<typename Op, typename... Ex>
  constexpr explicit expression_(Op&& op, Ex&&... ex) :
    Operator(std::forward<Op>(op)), std::tuple<Expressions...>(
                                      std::forward<Ex>(ex)...)
  {}

  constexpr decltype(auto) operator()() const
  {
    return deferred::apply(static_cast<Operator const&>(*this),
                           static_cast<subexpression_types const&>(*this));
  }

  constexpr decltype(auto) operator()()
  {
    return deferred::apply(static_cast<Operator&>(*this),
                           static_cast<subexpression_types&>(*this));
  }

  operator_type const& operator_() const noexcept
  {
    return static_cast<Operator const&>(*this);
  }

  decltype(auto) subexpressions() const noexcept
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
using make_callable_arg_t =
  std::conditional_t<is_deferred_v<T>, T, constant_<T>>;

} // namespace detail

/**
 * Transforms @p Callable into an @ref expression_ if it is not a
 * deferred data type.
 */
template<typename Callable, typename... Args>
using make_callable_t =
  std::conditional_t<is_expression_v<Callable>,
                     Callable,
                     expression_<make_function_object_t<Callable>,
                                 detail::make_callable_arg_t<Args>...>>;

/**
 * Transforms @p T into a @c deferred type.
 *
 * - If @p T is already a @c deferred type, it does not change.
 * - If @p T is a callable type, it is transformed to an @ref expression_.
 * - If @p T is not a callable type, it is transformed to an @ref constant_.
 */
template<typename T>
using make_deferred_t =
  std::conditional_t<is_deferred_v<T>,
                     T,
                     std::conditional_t<std::is_invocable_v<T>,
                                        expression_<make_function_object_t<T>>,
                                        constant_<T>>>;

} // namespace deferred

#endif
