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
 * @brief Checks evaluation and conversion to a mapped result type.
 * @tparam Result Target result type.
 * @tparam ExpressionReference Expression reference type.
 * @return @c true when evaluation and result construction cannot throw.
 */
template<typename Result, typename ExpressionReference>
consteval bool evaluation_result_is_nothrow()
{
  using mapped_result = map_void_t<decltype(evaluate(std::declval<ExpressionReference>()))>;
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
