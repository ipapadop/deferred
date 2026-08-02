// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_APPLY_HPP
#define DEFERRED_APPLY_HPP

#include <functional>
#include <tuple>
#include <utility>

namespace deferred {

namespace detail {

/**
 * @brief Invokes a callable with evaluated deferred arguments.
 * @tparam F Type of the callable.
 */
template<typename F>
struct apply_invoker
{
  F&& f;

  template<typename... Expressions>
  constexpr decltype(auto) operator()(Expressions&&... expressions) const
    noexcept(noexcept(std::invoke(std::forward<F>(f), std::forward<Expressions>(expressions)()...)))
  {
    return std::invoke(std::forward<F>(f), std::forward<Expressions>(expressions)()...);
  }
};

} // namespace detail

/**
 * @brief Invoke callable @p f with the tuple of @c deferred objects @p t.
 * @tparam F Type of the callable.
 * @tparam Tuple Type of the tuple containing deferred objects.
 * @param f The callable to invoke.
 * @param t The tuple containing deferred objects.
 * @return The result of invoking @p f with the evaluated elements of @p t.
 */
template<typename F, typename Tuple>
constexpr decltype(auto)
apply(F&& f, Tuple&& t) noexcept(noexcept(std::apply(detail::apply_invoker<F>{std::forward<F>(f)},
                                                     std::forward<Tuple>(t))))
{
  return std::apply(detail::apply_invoker<F>{std::forward<F>(f)}, std::forward<Tuple>(t));
}

} // namespace deferred

#endif
