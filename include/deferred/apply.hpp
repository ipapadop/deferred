// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_APPLY_HPP
#define DEFERRED_APPLY_HPP

#include <tuple>
#include <utility>

#include "invoke.hpp"

namespace deferred {

namespace detail {

/**
 * @brief Builds a deferred invocation from unpacked tuple elements.
 * @tparam F Type of the callable.
 */
template<typename F>
struct invoke_builder
{
  F&& f;

  template<typename... Args>
  constexpr auto operator()(Args&&... args) const
    noexcept(noexcept(deferred::invoke(std::forward<F>(f), std::forward<Args>(args)...)))
  {
    return deferred::invoke(std::forward<F>(f), std::forward<Args>(args)...);
  }
};

} // namespace detail

/**
 * @brief Creates a deferred invocation of @p f with the elements of the tuple @p t.
 *
 * This is the tuple form of @ref invoke(): <tt>apply(f, t)</tt> is equivalent to
 * <tt>invoke(f, std::get<I>(t)...)</tt>, mirroring the relationship between
 * @c std::apply and @c std::invoke. Nothing is evaluated; the returned expression
 * is evaluated later like any other @c deferred object.
 *
 * @tparam F Type of the callable.
 * @tparam Tuple Type of the tuple containing the arguments.
 * @param f The callable to invoke.
 * @param t The tuple containing the arguments.
 * @return An expression representing the invocation.
 */
template<typename F, typename Tuple>
[[nodiscard]] constexpr auto
apply(F&& f, Tuple&& t) noexcept(noexcept(std::apply(detail::invoke_builder<F>{std::forward<F>(f)},
                                                     std::forward<Tuple>(t))))
{
  return std::apply(detail::invoke_builder<F>{std::forward<F>(f)}, std::forward<Tuple>(t));
}

} // namespace deferred

#endif
