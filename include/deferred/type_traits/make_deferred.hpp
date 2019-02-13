/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TYPE_TRAITS_MAKE_DEFERRED_HPP
#define DEFERRED_TYPE_TRAITS_MAKE_DEFERRED_HPP

#include <type_traits>

#include "is_constant.hpp"
#include "is_constant_expression.hpp"
#include "is_expression.hpp"

namespace deferred
{

/// Transforms @p T into a @c deferred data type.
template<typename T>
using make_deferred_t =
  std::conditional_t<
    is_deferred_v<std::decay_t<T>>,
    T,
    std::conditional_t<
      is_constant_expression_v<std::decay_t<T>>,
      constant_<T>,
      expression_<T>>
  >;

} // namespace deferred

#endif
