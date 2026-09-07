// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_EXPRESSION_REFERENCE_HPP
#define DEFERRED_DETAIL_EXPRESSION_REFERENCE_HPP

#include <type_traits>

namespace deferred::detail {

/**
 * @brief Selects a const or mutable expression reference from @p Self.
 * @tparam Self Enclosing expression reference type.
 * @tparam Expression Child expression type.
 */
template<typename Self, typename Expression>
using expression_reference_t =
  std::conditional_t<std::is_const_v<std::remove_reference_t<Self>>, Expression const&, Expression&>;

} // namespace deferred::detail

#endif
