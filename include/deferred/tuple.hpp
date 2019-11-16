/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TUPLE_HPP
#define DEFERRED_TUPLE_HPP

#include <tuple>
#include <type_traits>
#include <utility>

namespace deferred {

namespace detail {

template<typename T, typename F, std::size_t... Is>
constexpr void for_each(T&& t, F&& f, std::index_sequence<Is...>)
{
  using array_type = int[];
  (void)array_type{(std::forward<F>(f)(std::get<Is>(t)), 0)...};
}

} // namespace detail

/// Calls @p f on each element of the tuple @p t.
template<typename... Ts, typename F>
constexpr void for_each(std::tuple<Ts...> const& t, F&& f)
{
  detail::for_each(t, std::forward<F>(f), std::index_sequence_for<Ts...>{});
}

} // namespace deferred

#endif
