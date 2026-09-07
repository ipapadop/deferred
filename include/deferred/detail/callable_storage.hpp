// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_CALLABLE_STORAGE_HPP
#define DEFERRED_DETAIL_CALLABLE_STORAGE_HPP

#include <type_traits>

namespace deferred::detail {

/**
 * @brief Type used to hold a forwarded callable inside a function object.
 *
 * Callables are held by reference, except functions, which are held as function
 * pointers: MSVC cannot read a member that is a reference to a function in a
 * constant expression.
 *
 * @tparam F Forwarded callable type.
 */
template<typename F>
using callable_storage_t = std::conditional_t<std::is_function_v<std::remove_reference_t<F>>,
                                              std::add_pointer_t<std::remove_reference_t<F>>,
                                              F&&>;

} // namespace deferred::detail

#endif
