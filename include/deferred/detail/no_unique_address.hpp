// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_NO_UNIQUE_ADDRESS_HPP
#define DEFERRED_DETAIL_NO_UNIQUE_ADDRESS_HPP

/**
 * @def DEFERRED_NO_UNIQUE_ADDRESS
 * @brief Lets an empty member occupy no storage.
 *
 * The Microsoft ABI cannot change the layout of existing types, so MSVC and clang-cl
 * accept the standard @c [[no_unique_address]] but ignore it, and provide
 * @c [[msvc::no_unique_address]] instead. The choice follows the ABI rather than the
 * compiler, so it is made by attribute detection: clang-cl reports @c __clang__ yet needs
 * the Microsoft spelling.
 *
 * Expands to nothing where neither spelling is available: the library stays correct, but
 * empty subexpressions cost storage. @c test/unit/detail/no_unique_address.cpp fails to
 * compile in that case, so it does not pass unnoticed.
 */
#if defined(__has_cpp_attribute)
#  if __has_cpp_attribute(msvc::no_unique_address)
#    define DEFERRED_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#  elif __has_cpp_attribute(no_unique_address)
#    define DEFERRED_NO_UNIQUE_ADDRESS [[no_unique_address]]
#  endif
#endif

#ifndef DEFERRED_NO_UNIQUE_ADDRESS
#  define DEFERRED_NO_UNIQUE_ADDRESS
#endif

#endif
