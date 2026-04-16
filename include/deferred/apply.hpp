// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_APPLY_HPP
#define DEFERRED_APPLY_HPP

#include <tuple>
#include <type_traits>
#include <utility>

namespace deferred {

namespace detail {

/**
 * @brief Helper function to apply a callable to the elements of a tuple.
 * @copydoc apply(F&&, Tuple&&)
 * @tparam I Index sequence for tuple elements.
 * @param I Index sequence for tuple elements.
 */
template<typename F, typename Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
{
  return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))()...);
}

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
constexpr decltype(auto) apply(F&& f, Tuple&& t)
{
  return detail::apply_impl(
    std::forward<F>(f),
    std::forward<Tuple>(t),
    std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

} // namespace deferred

#endif
