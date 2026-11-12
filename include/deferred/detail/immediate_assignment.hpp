// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_IMMEDIATE_ASSIGNMENT_HPP
#define DEFERRED_DETAIL_IMMEDIATE_ASSIGNMENT_HPP

namespace deferred::detail {

/**
 * @brief Result of assigning a value to a variable, which stores the value at once.
 *
 * It carries nothing and is not a @c deferred expression. Its purpose is to be a type
 * that no expression accepts: an assignment that has already happened cannot be a loop
 * clause, a branch or a body, and returning the variable instead would let it pass as
 * one that merely reads the variable.
 */
struct immediate_assignment
{ };

} // namespace deferred::detail

#endif
