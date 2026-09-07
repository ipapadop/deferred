// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_IS_NOTHROW_EVALUABLE_HPP
#define DEFERRED_DETAIL_IS_NOTHROW_EVALUABLE_HPP

namespace deferred::detail {

/**
 * @brief Describes whether an expression reference can be evaluated without throwing.
 *
 * Specialized by each control-flow expression that computes its own guarantee.
 *
 * @tparam ExpressionReference Expression reference type.
 */
template<typename ExpressionReference>
struct is_nothrow_evaluable;

/**
 * @brief Whether an expression reference can be evaluated without throwing.
 * @tparam ExpressionReference Expression reference type.
 */
template<typename ExpressionReference>
inline constexpr bool is_nothrow_evaluable_v = is_nothrow_evaluable<ExpressionReference>::value;

} // namespace deferred::detail

#endif
