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

#include <functional>
#include <utility>

#include "expression.hpp"

namespace deferred {

namespace detail {

template<typename F, typename... Args>
using returns_deferred_expression_t =
  is_deferred_t<decltype(std::declval<F>()(std::declval<Args>()...))>;

template<typename F,
         typename... Args,
         typename std::enable_if_t<
           !returns_deferred_expression_t<F, Args...>{}>* = nullptr>
constexpr decltype(auto) invoke_immediate(F&& f, Args&&... args)
{
  return std::forward<F>(f)(std::forward<Args>(args)...);
}

template<typename F,
         typename... Args,
         typename std::enable_if_t<
           returns_deferred_expression_t<F, Args...>{}>* = nullptr>
constexpr decltype(auto) invoke_immediate(F&& f, Args&&... args)
{
  return invoke_immediate(std::forward<F>(f)(), std::forward<Args>(args)...);
}

} // namespace detail

/**
 * Invoke the callable object @p f with the parameters @p args....
 */
template<typename F, typename... Args>
constexpr decltype(auto) invoke_immediate(F&& f, Args&&... args)
{
  return detail::invoke_immediate(std::forward<F>(f),
                                  std::forward<Args>(args)...);
}

/**
 * Created a @c deferred expression that its evaluation will invoke the callable
 * object @p f with the parameters @p args....
 *
 * If @p args... are not @c deferred objects, then they will be tranformed to
 * @ref constant_ objects.
 */
template<typename F, typename... Args>
constexpr auto invoke(F&& f, Args&&... args)
{
  using expression_type = make_callable_t<F, Args...>;
  return expression_type(std::forward<F>(f), std::forward<Args>(args)...);
}

} // namespace deferred

#endif

