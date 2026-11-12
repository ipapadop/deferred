// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_DETAIL_EVAL_MIXIN_HPP
#define DEFERRED_DETAIL_EVAL_MIXIN_HPP

#include <utility>

namespace deferred::detail {

/**
 * @brief Gives an expression node @c eval() , a named spelling of its <tt>operator()</tt>.
 *
 * A node derives from <tt>eval_mixin<node></tt>. @c eval() forwards to the call
 * operator of the same constness, so its result type and exception specification are
 * those of the operator and cannot differ from them. Like the operator, it is not
 * @c [[nodiscard]] : an expression may be evaluated only for its effect.
 *
 * @tparam Derived The node type deriving from this class.
 */
template<typename Derived>
class eval_mixin
{
public:
  /**
   * @brief Evaluates the expression.
   * @return The result of calling the expression.
   */
  constexpr decltype(auto) eval() const noexcept(noexcept(std::declval<Derived const&>()()))
  {
    return static_cast<Derived const&>(*this)();
  }

  /// @copydoc eval_mixin::eval() const
  constexpr decltype(auto) eval() noexcept(noexcept(std::declval<Derived&>()()))
  {
    return static_cast<Derived&>(*this)();
  }
};

} // namespace deferred::detail

#endif
