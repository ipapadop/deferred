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

#include "expression.hpp"

namespace deferred
{

namespace detail
{

// Creates an expression_ from any callable
template<typename F, typename... Args>
constexpr auto invoke(F&& f, Args&&... args)
{
  using expression_type = make_expression_t<F, Args...>;
  return expression_type(std::forward<F>(f), std::forward<Args>(args)...);
}

// Wraps a function pointer
template<typename F>
struct fun_ptr_wrapper
{
  F m_f;

  constexpr fun_ptr_wrapper(F f) noexcept
    : m_f{f}
  {}

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
  using operator_type   = fun_ptr_wrapper<T*>;
  using expression_type = make_expression_t<operator_type, Args...>;
  return expression_type(f, std::forward<Args>(args)...);
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
