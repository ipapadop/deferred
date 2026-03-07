/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TUPLE_HPP
#define DEFERRED_TUPLE_HPP

#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace deferred {

/// Calls @p f on each element of the tuple @p t.
template<typename Tuple, typename F>
constexpr void for_each(Tuple&& t, F&& f)
{
  std::apply([&f](auto&&... args) { (std::invoke(f, std::forward<decltype(args)>(args)), ...); },
             std::forward<Tuple>(t));
}

} // namespace deferred

#endif
