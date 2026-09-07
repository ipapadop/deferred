// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_INVOKE_HPP
#define DEFERRED_INVOKE_HPP

#include <utility>

#include "expression.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

namespace detail {

/**
 * @brief Expression type produced by a deferred invocation.
 * @tparam F Callable argument type.
 * @tparam Args Invocation argument types.
 */
template<typename F, typename... Args>
using invoke_expression_t = expression_<std::decay_t<F>, make_deferred_t<Args>...>;

/**
 * @brief Checks whether constructing a deferred invocation cannot throw.
 * @tparam F Callable argument type.
 * @tparam Args Invocation argument types.
 * @return @c true when storing the callable and arguments cannot throw.
 */
template<typename F, typename... Args>
consteval bool invoke_is_nothrow()
{
  if constexpr (Deferred<F>)
  {
    return std::is_nothrow_constructible_v<std::decay_t<F>, F&&>;
  }
  else
  {
    return std::is_nothrow_constructible_v<invoke_expression_t<F, Args...>, F&&, Args&&...>;
  }
}

} // namespace detail

/**
 * @brief Invoke the callable object @p f with the parameters @p args....
 *
 * If @p args... are not @c deferred objects, then they will be tranformed to
 * @ref constant_ objects.
 *
 * @tparam F Type of the callable object.
 * @tparam Args Types of the parameters.
 * @param f Callable object.
 * @param args Parameters to pass to the callable object.
 * @return An expression representing the invocation.
 */
template<typename F, typename... Args>
[[nodiscard]] constexpr auto
invoke(F&& f, Args&&... args) noexcept(detail::invoke_is_nothrow<F, Args...>())
{
  if constexpr (Deferred<F>)
  {
    static_assert(sizeof...(Args) == 0);
    return std::forward<F>(f);
  }
  else
  {
    return detail::invoke_expression_t<F, Args...>(std::forward<F>(f), std::forward<Args>(args)...);
  }
}

} // namespace deferred

#endif
