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
#include "type_traits/is_deferred.hpp"

namespace deferred {

namespace detail {

// Transforms T into a constant_ if it is not a deferred data type.
template<typename T>
using make_callable_arg_t =
  std::conditional_t<is_deferred_v<T>, T, constant_<std::decay_t<T>>>;

} // namespace detail

/**
 * Invoke the callable object @p f with the parameters @p args....
 *
 * If @p args... are not @c deferred objects, then they will be tranformed to
 * @ref constant_ objects.
 */
template<typename F, typename... Args>
constexpr auto invoke(F&& f, Args&&... args)
{
  if constexpr (sizeof...(Args) == 0)
  {
    // deferred expressions do not accept arguments
    using expression_type = std::conditional_t<
      is_deferred_v<F>,
      F,
      expression_<make_function_object_t<std::remove_reference_t<F>>>>;
    return expression_type(std::forward<F>(f));
  }
  else
  {
    using expression_type =
      expression_<make_function_object_t<std::remove_reference_t<F>>,
                  detail::make_callable_arg_t<Args>...>;
    return expression_type(std::forward<F>(f), std::forward<Args>(args)...);
  }
}

} // namespace deferred

#endif
