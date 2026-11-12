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

  static_assert(!std::is_reference_v<T>,
                "a constant owns its value, so T cannot be a reference; write constant(x) and "
                "let T be deduced");

private:
  DEFERRED_NO_UNIQUE_ADDRESS T m_t;

public:
  /**
   * @brief Constructs a constant holding a copy of @p t.
   *
   * The parameter does not take part in deducing @p T , so that a deduced
   * <tt>constant(x)</tt> always goes through the deduction guide.
   *
   * @param t Value to store.
   */
  [[nodiscard]] constexpr explicit constant(std::type_identity_t<T> const& t) //
    noexcept(std::is_nothrow_copy_constructible_v<T>) : m_t(t)
  { }

  /**
   * @brief Constructs a constant holding @p t , moved.
   * @param t Value to store.
   */
  [[nodiscard]] constexpr explicit constant(std::type_identity_t<T>&& t) //
    noexcept(std::is_nothrow_move_constructible_v<T>) : m_t(std::move(t))
  { }

  /**
   * @brief Constructs a constant from the result of evaluating @p u.
   *
   * Chosen when @p u is callable with no arguments and the result of evaluating it
   * converts to @p T . It is called, and the result called again until it is not
   * callable; this applies to a @c deferred expression too. An argument that is not
   * callable, or whose result is not a @p T , is stored by the constructors above.
   *
   * @tparam U Type of the callable.
   * @param u Callable yielding the value to store.
   */
  template<typename U>
    requires(!std::same_as<std::remove_cvref_t<U>, constant>) && detail::evaluates_to<U, T>
  [[nodiscard]] constexpr explicit constant(U&& u) noexcept(
    noexcept(recursive_evaluate(std::declval<U>()))
    && std::is_nothrow_constructible_v<T, detail::node_result_t<U>>) :
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
