// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_APPLY_EVALUATED_HPP
#define DEFERRED_DETAIL_APPLY_EVALUATED_HPP

#include <functional>
#include <tuple>
#include <utility>

#include "callable_storage.hpp"

namespace deferred::detail {

/**
 * @brief Invokes a callable with evaluated deferred arguments.
 *
 * Implementation detail of @ref apply_evaluated(); it is the function object
 * handed to @c std::apply.
 *
 * @tparam F Type of the callable.
 */
template<typename F>
struct apply_invoker
{
  callable_storage_t<F> f;

  template<typename... Expressions>
  constexpr decltype(auto) operator()(Expressions&&... expressions) const
    noexcept(noexcept(std::invoke(std::declval<callable_storage_t<F>>(),
                                  std::forward<Expressions>(expressions)()...)))
  {
    return std::invoke(static_cast<callable_storage_t<F>>(f),
                       std::forward<Expressions>(expressions)()...);
  }
};

/**
 * @brief Invoke callable @p f with the evaluated elements of the tuple @p t.
 *
 * Each element of @p t is a @c deferred object; it is evaluated once through
 * <tt>element()</tt> and the results are passed to @p f. This is the step that
 * runs a deferred expression, and is the opposite of @ref deferred::apply(),
 * which builds one.
 *
 * @tparam F Type of the callable.
 * @tparam Tuple Type of the tuple containing deferred objects.
 * @param f The callable to invoke.
 * @param t The tuple containing deferred objects.
 * @return The result of invoking @p f with the evaluated elements of @p t.
 */
template<typename F, typename Tuple>
constexpr decltype(auto)
apply_evaluated(F&& f, Tuple&& t) noexcept(noexcept(std::apply(apply_invoker<F>{std::forward<F>(f)},
                                                               std::forward<Tuple>(t))))
{
  return std::apply(apply_invoker<F>{std::forward<F>(f)}, std::forward<Tuple>(t));
}

} // namespace deferred::detail

#endif
