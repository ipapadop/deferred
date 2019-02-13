/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_INVOKE_HPP
#define DEFERRED_INVOKE_HPP

#include <type_traits>
#include <utility>

#include "constant.hpp"
#include "expression.hpp"

namespace deferred
{

namespace detail
{

// Transforms T into an expression_ if it is not a deferred data type.
template<typename T, typename... Args>
using make_invocable_expression_t =
  std::conditional_t<
    is_expression_v<std::decay_t<T>>,
    T,
    expression_<T, make_deferred_t<Args>...>>;


// Transforms function pointer F to a function object
template<typename F>
struct fun_ptr
{
  F m_f;

  template<typename... T>
  constexpr decltype(auto) operator()(T&&... t) const
  {
    return m_f(std::forward<T>(t)...);
  }
};

// Creates an expression_ from a function pointer
template<typename T, typename... Args>
constexpr auto invoke(T* f, Args&&... args)
{
  using fun_ptr_t = fun_ptr<T*>;
  using expression_type = expression_<fun_ptr_t, make_deferred_t<Args>...>;
  return expression_type(fun_ptr_t{f}, std::forward<Args>(args)...);
}

// Creates an expression_ from any callable
template<typename F, typename... Args>
constexpr auto invoke(F&& f, Args&&... args)
{
  using expression_type = detail::make_invocable_expression_t<F, Args...>;
  return expression_type(std::forward<F>(f), std::forward<Args>(args)...);
}

} // namespace detail

/// Invoke the callable object @p f with the parameters @p args....
template<typename F, typename... Args>
constexpr auto invoke(F&& f, Args&&... args)
{
  return detail::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

} // namespace deferred

#endif
