// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_NO_MATCH_IS_NOTHROW_HPP
#define DEFERRED_DETAIL_NO_MATCH_IS_NOTHROW_HPP

#include <optional>
#include <type_traits>

namespace deferred::detail {

/**
 * @brief Whether reporting that no branch matched cannot throw.
 *
 * Counterpart of @ref unmatched_result_t: a non-finalized control-flow expression
 * that matches nothing returns either @c void or an empty @c std::optional, so this
 * is the exception guarantee of that path.
 *
 * @tparam Result Result type of the control-flow expression.
 */
template<typename Result>
inline constexpr bool no_match_is_nothrow_v =
  std::is_void_v<Result> || std::is_nothrow_constructible_v<Result, std::nullopt_t>;

} // namespace deferred::detail

#endif
