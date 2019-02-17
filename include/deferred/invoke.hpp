/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_INVOKE_HPP
#define DEFERRED_INVOKE_HPP

#include <utility>

#include "expression.hpp"

namespace deferred {

/**
 * Invoke the callable object @p f with the parameters @p args....
 *
 * If @p args... are not @c deferred objects, then they will be tranformed to
 * @ref constant_ objects.
 */
template<typename F, typename... Args>
constexpr auto invoke(F&& f, Args&&... args)
{
  using expression_type = make_expression_t<F, Args...>;
  return expression_type(std::forward<F>(f), std::forward<Args>(args)...);
}

} // namespace deferred

#endif
