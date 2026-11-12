// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_FORWARDED_PAIR_HPP
#define DEFERRED_DETAIL_FORWARDED_PAIR_HPP

namespace deferred::detail {

/**
 * @brief Two forwarded arguments, carried as one.
 *
 * A @c std::tuple constructs each element from a single argument. A node made of two
 * expressions -- a branch, a case -- takes a @c forwarded_pair so that a tuple of such
 * nodes can construct one in place, rather than from a temporary node that is then
 * moved.
 *
 * It is an aggregate of two references and must not outlive the full expression that
 * creates it. A @c std::tuple of references does the same job, but GCC 15 at @c -O2 then
 * evaluates the finished expression several times slower.
 *
 * @tparam First Type of the first argument.
 * @tparam Second Type of the second argument.
 */
template<typename First, typename Second>
struct forwarded_pair
{
  First&& first;
  Second&& second;
};

} // namespace deferred::detail

#endif
