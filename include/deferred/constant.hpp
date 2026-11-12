// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_CONSTANT_HPP
#define DEFERRED_CONSTANT_HPP

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/is_nothrow_visitable.hpp"
#include "detail/no_unique_address.hpp"
#include "type_traits/expression_visitor.hpp"

#include "evaluate.hpp"

namespace deferred {

/**
 * @brief Stores a constant value.
 *
 * The type is usually deduced: <tt>constant c{42};</tt> or <tt>auto c = constant(42);</tt>.
 *
 * @tparam T Type of the constant.
 */
template<typename T>
class constant
{
public:
  using value_type          = T;
  using subexpression_types = std::tuple<>;

private:
  DEFERRED_NO_UNIQUE_ADDRESS T m_t;

public:
  /**
   * @brief Constructs a constant from @p u.
   *
   * If @p u is callable with no arguments, it is called and the result is stored; this
   * repeats until the result is not callable, and applies to a @c deferred expression
   * too. A constant therefore never holds something it could evaluate.
   *
   * @tparam U Type of the value to initialize the constant with.
   * @param u Value, or callable yielding the value, to initialize the constant with.
   */
  template<typename U>
    requires std::convertible_to<decltype(recursive_evaluate(std::declval<U>())), T>
  [[nodiscard]] constexpr explicit constant(U&& u) noexcept(
    noexcept(recursive_evaluate(std::declval<U>()))
    && std::is_nothrow_constructible_v<T, decltype(recursive_evaluate(std::declval<U>()))>) :
    m_t(recursive_evaluate(std::forward<U>(u)))
  { }

  constant(constant const&) = default;
  constant(constant&&)      = default;

  constant& operator=(constant const&) = delete;
  constant& operator=(constant&&)      = delete;

  /// @brief Returns the stored value.
  [[nodiscard]] constexpr T const& operator()() const& noexcept
  {
    return m_t;
  }

  /// @copydoc operator()()
  [[nodiscard]] constexpr T operator()() && noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    return std::move(m_t);
  }

  /**
   * @brief Returns the stored value.
   *
   * A named spelling of <tt>operator()</tt>, for code that reads the value of the
   * constant rather than evaluating it as an expression.
   */
  [[nodiscard]] constexpr T const& get() const& noexcept
  {
    return (*this)();
  }

  /// @brief Returns the stored value, moved out of the constant.
  [[nodiscard]] constexpr T get() && noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    return std::move(*this)();
  }

  /**
   * @brief Returns references to the subexpressions.
   * @return A @c std::tuple of references, in @c subexpression_types order.
   */
  [[nodiscard]] constexpr auto subexpressions() const noexcept
  {
    return std::tuple<>{};
  }

  /**
   * @brief Visits the constant with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
    requires expression_visitor<Visitor, constant>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, constant, subexpression_types>)
  {
    v(*this, nesting);
  }
};

/**
 * @brief Deduces the type of a constant from the value it is constructed with.
 *
 * The stored type is the decayed type of the value, so an array is held as a pointer
 * and a reference is never retained. A callable is evaluated first, as the constructor
 * does, so <tt>constant([] { return 1; })</tt> is a <tt>constant<int></tt>.
 *
 * @tparam T Type of the value to wrap.
 */
template<typename T>
constant(T&&) -> constant<detail::node_result_t<T&&>>;

} // namespace deferred

#endif
