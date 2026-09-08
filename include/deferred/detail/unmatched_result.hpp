// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_UNMATCHED_RESULT_HPP
#define DEFERRED_DETAIL_UNMATCHED_RESULT_HPP

#include <optional>
#include <type_traits>

namespace deferred::detail {

/**
 * @brief Result type of a control-flow expression that may match no branch.
 *
 * A finalized expression -- one with an @c else branch or a @c default case --
 * always produces a value, so its result is @p Base. A non-finalized one may match
 * nothing, which is reported as an empty @c std::optional, or as @c void when the
 * branches produce no value at all.
 *
 * @tparam Finalized Whether the expression always matches.
 * @tparam Base Result type of the branches taken together.
 */
template<bool Finalized, typename Base>
using unmatched_result_t =
  std::conditional_t<Finalized,
                     Base,
                     std::conditional_t<std::is_void_v<Base>, void, std::optional<Base>>>;

} // namespace deferred::detail

#endif
