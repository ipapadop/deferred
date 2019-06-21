/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_APPLY_HPP
#define DEFERRED_APPLY_HPP

#include <tuple>
#include <type_traits>
#include <utility>

namespace deferred {

namespace detail {

template<typename F, typename Tuple, std::size_t... I>
constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
{
  return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))()...);
}

} // namespace detail

/// Invoke callable @p f with the tuple of @c deferred objects @p t.
template<typename F, typename Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t)
{
  return detail::apply_impl(
    std::forward<F>(f),
    std::forward<Tuple>(t),
    std::make_index_sequence<
      std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

} // namespace deferred

#endif

