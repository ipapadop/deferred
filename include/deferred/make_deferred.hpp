/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_MAKE_DEFERRED_HPP
#define DEFERRED_MAKE_DEFERRED_HPP

#include <type_traits>

#include "constant.hpp"
#include "expression.hpp"
#include "type_traits.hpp"

namespace deferred
{

// Transforms T into a deferred data type.
template<typename T>
using make_deferred_t =
  std::conditional_t<
    is_deferred_v<std::decay_t<T>>,
    T,
    std::conditional_t<
      std::is_invocable_v<T> || std::is_function_v<T>,
      expression_<T>,
      constant_<T>
    >
  >;

} // namespace deferred

#endif
