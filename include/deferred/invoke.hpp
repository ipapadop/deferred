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

#include <utility>

#include "expression.hpp"

namespace deferred {

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
 * Invoke the callable object @p f with the parameters @p args....
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
