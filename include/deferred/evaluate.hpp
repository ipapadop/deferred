// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_EVALUATE_HPP
#define DEFERRED_EVALUATE_HPP

#include <type_traits>
#include <utility>

#include "detail/map_void.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

namespace detail {

/**
 * @brief Checks whether evaluating a value cannot throw.
 * @tparam T Value, deferred expression or callable type.
 * @tparam Recursive Whether every callable is invoked, not only deferred expressions.
 * @return @c true when evaluation cannot throw.
 */
template<typename T, bool Recursive>
consteval bool evaluation_is_nothrow()
{
  if constexpr (Recursive ? std::is_invocable_v<T> : Deferred<T>)
  {
    if constexpr (!std::is_nothrow_invocable_v<T>)
    {
      return false;
    }
    else if constexpr (std::is_void_v<std::invoke_result_t<T>>)
    {
      return true;
    }
    else
    {
      return evaluation_is_nothrow<std::invoke_result_t<T>, Recursive>();
    }
  }
  else
  {
    return std::is_nothrow_constructible_v<std::decay_t<T>, T&&>;
  }
}

} // namespace detail

/**
 * @brief Evaluates a deferred expression.
 * @tparam T The type of the expression to evaluate.
 * @param t The expression to evaluate.
 * @return The result of evaluating the expression.
 */
template<typename T>
constexpr auto evaluate(T&& t) noexcept(detail::evaluation_is_nothrow<T, false>())
{
  if constexpr (Deferred<T>)
  {
    if constexpr (std::is_void_v<decltype(std::forward<T>(t)())>)
    {
      std::forward<T>(t)();
    }
    else
    {
      return evaluate(std::forward<T>(t)());
    }
  }
  else
  {
    return std::forward<T>(t);
  }
}

namespace detail {

/**
 * @brief Checks whether evaluating a value without decaying it cannot throw.
 * @tparam T Value or deferred expression type.
 * @return @c true when evaluation cannot throw.
 */
template<typename T>
consteval bool reference_evaluation_is_nothrow()
{
  if constexpr (!Deferred<T>)
  {
    return true;
  }
  else if constexpr (!std::is_nothrow_invocable_v<T>)
  {
    return false;
  }
  else if constexpr (std::is_void_v<std::invoke_result_t<T>>)
  {
    return true;
  }
  else if constexpr (Deferred<std::invoke_result_t<T>>)
  {
    return evaluation_is_nothrow<std::invoke_result_t<T>, false>();
  }
  else if constexpr (std::is_reference_v<std::invoke_result_t<T>>)
  {
    return true;
  }
  else
  {
    return std::is_nothrow_constructible_v<std::decay_t<std::invoke_result_t<T>>,
                                           std::invoke_result_t<T>>;
  }
}

} // namespace detail

/**
 * @brief Evaluates a deferred expression, keeping a reference result as a reference.
 *
 * @ref evaluate() returns by value, so a subexpression that already yields a reference --
 * a @ref variable_ or a @ref constant_ -- is copied. Where the result is only read within
 * the same full expression, such as a loop condition or the right-hand side of an
 * assignment, that copy is pure waste and this avoids it.
 *
 * A result that is itself a deferred expression is a temporary, so it falls back to
 * @ref evaluate() rather than returning a reference into it.
 *
 * @tparam T The type of the expression to evaluate.
 * @param t The expression to evaluate.
 * @return The result of evaluating the expression, as a reference where there is one.
 */
template<typename T>
constexpr decltype(auto) evaluate_ref(T&& t) noexcept(detail::reference_evaluation_is_nothrow<T>())
{
  if constexpr (!Deferred<T>)
  {
    return std::forward<T>(t);
  }
  else
  {
    using result_type = decltype(std::forward<T>(t)());
    if constexpr (std::is_void_v<result_type>)
    {
      std::forward<T>(t)();
    }
    else if constexpr (Deferred<result_type>)
    {
      return evaluate(std::forward<T>(t)());
    }
    else
    {
      return static_cast<result_type>(std::forward<T>(t)());
    }
  }
}

/**
 * @brief Recursively evaluates @p t until a non-callable is returned.
 * @copydoc evaluate(T&&)
 */
template<typename T>
constexpr auto recursive_evaluate(T&& t) noexcept(detail::evaluation_is_nothrow<T, true>())
{
  if constexpr (std::is_invocable_v<T>)
  {
    if constexpr (std::is_void_v<decltype(std::forward<T>(t)())>)
    {
      std::forward<T>(t)();
    }
    else
    {
      return recursive_evaluate(std::forward<T>(t)());
    }
  }
  else
  {
    return std::forward<T>(t);
  }
}

namespace detail {

/**
 * @brief Type an expression yields when it is evaluated.
 *
 * A deferred expression may return another deferred expression; @ref evaluate()
 * keeps evaluating until a value is produced, so this is the type a control-flow
 * expression actually observes for a subexpression, and it is never a reference.
 *
 * @tparam Expression Expression type.
 */
template<typename Expression>
using evaluated_result_t = decltype(evaluate(std::declval<Expression>()));

/**
 * @brief Checks evaluation and conversion to a mapped result type.
 * @tparam Result Target result type.
 * @tparam ExpressionReference Expression reference type.
 * @return @c true when evaluation and result construction cannot throw.
 */
template<typename Result, typename ExpressionReference>
consteval bool evaluation_result_is_nothrow()
{
  using mapped_result = map_void_t<evaluated_result_t<ExpressionReference>>;
  return noexcept(evaluate(std::declval<ExpressionReference>()))
         && std::is_nothrow_constructible_v<Result, mapped_result>;
}

/**
 * @brief Result type stored by a leaf node created from an argument of type @p T.
 * @tparam T Node argument type.
 */
template<typename T>
using node_result_t = std::decay_t<decltype(recursive_evaluate(std::declval<T>()))>;

/**
 * @brief Checks whether creating a leaf node from an argument cannot throw.
 * @tparam Node Leaf node template, such as @ref constant_ or @ref variable_.
 * @tparam T Node argument type.
 * @return @c true when the value can be stored without throwing.
 */
template<template<typename> class Node, typename T>
consteval bool make_node_is_nothrow()
{
  using evaluated_type = decltype(recursive_evaluate(std::declval<T>()));
  return noexcept(recursive_evaluate(std::declval<T>()))
         && std::is_nothrow_constructible_v<Node<node_result_t<T>>, evaluated_type>;
}

} // namespace detail

} // namespace deferred

#endif
