// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_INVOKE_HPP
#define DEFERRED_INVOKE_HPP

#include <utility>

#include "expression.hpp"
#include "make_function_object.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

/**
 * @brief Invoke the callable object @p f with the parameters @p args....
 *
 * If @p args... are not @c deferred objects, then they will be tranformed to
 * @ref constant_ objects.
 *
 * @tparam F The type of the callable object.
 * @tparam Args The types of the parameters.
 * @param f The callable object.
 * @param args The parameters to pass to the callable object.
 * @return An expression representing the invocation.
 */
template<typename F, typename... Args>
[[nodiscard]] constexpr auto invoke(F&& f, Args&&... args)
{
  if constexpr (is_deferred_v<std::remove_reference_t<F>>)
  {
    static_assert(sizeof...(Args) == 0);
    return std::forward<F>(f);
  }
  else
  {
    using expression_type =
      expression_<std::decay_t<decltype(make_function_object(std::forward<F>(f)))>,
                  make_deferred_t<Args>...>;
    return expression_type(std::forward<F>(f), std::forward<Args>(args)...);
  }
}

} // namespace deferred

#endif
