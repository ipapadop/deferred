// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_VARIABLE_HPP
#define DEFERRED_VARIABLE_HPP

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/immediate_assignment.hpp"
#include "detail/is_nothrow_visitable.hpp"
#include "detail/no_unique_address.hpp"
#include "type_traits/expression_visitor.hpp"

#include "assign.hpp"
#include "evaluate.hpp"

namespace deferred {

/**
 * @brief Stores a variable value.
 *
 * The type is usually deduced: <tt>variable v{42};</tt> or <tt>auto v = variable(42);</tt>.
 * A variable with no initial value names its type: <tt>variable<int> v;</tt>.
 *
 * @tparam T Type of the variable to store.
 */
template<typename T>
class [[nodiscard]] variable
{
public:
  using value_type          = T;
  using subexpression_types = std::tuple<>;

  /// @brief The stored value can change between evaluations.
  static constexpr bool mutable_state = true;

  static_assert(!std::is_reference_v<T>,
                "a variable owns its value, so T cannot be a reference; write variable(x) and "
                "let T be deduced");

private:
  DEFERRED_NO_UNIQUE_ADDRESS T m_t{};

public:
  variable() = default;

  /**
   * @brief Constructs a variable holding a copy of @p t.
   *
   * The parameter does not take part in deducing @p T , so that a deduced
   * <tt>variable(x)</tt> always goes through the deduction guide.
   *
   * @param t Value to store.
   */
  constexpr explicit variable(std::type_identity_t<T> const& t) //
    noexcept(std::is_nothrow_copy_constructible_v<T>) : m_t(t)
  { }

  /**
   * @brief Constructs a variable holding @p t , moved.
   * @param t Value to store.
   */
  constexpr explicit variable(std::type_identity_t<T>&& t) //
    noexcept(std::is_nothrow_move_constructible_v<T>) : m_t(std::move(t))
  { }

  /**
   * @brief Constructs a variable from the result of evaluating @p u.
   *
   * Chosen when @p u is callable with no arguments and the result of evaluating it
   * converts to @p T . It is called, and the result called again until it is not
   * callable; this applies to a @c deferred expression too. An argument that is not
   * callable, or whose result is not a @p T , is stored by the constructors above.
   *
   * When evaluating @p u yields an lvalue, @p T is constructed from that lvalue and
   * not from a copy of it. A view type such as @c std::string_view therefore views
   * the value the source holds, which must then outlive this node.
   *
   * A variable is such an expression, so <tt>variable w{v};</tt> makes a new,
   * independent variable holding the current value of @c v . The source must be a
   * non-const lvalue: a const or temporary variable selects the deleted copy or move
   * constructor instead, and <tt>variable w{v()};</tt> is the spelling for those.
   *
   * @tparam U Type of the callable.
   * @param u Callable yielding the value to store.
   */
  template<typename U>
    requires(!std::same_as<std::remove_cvref_t<U>, variable>
             || (std::same_as<U, variable&> && std::copy_constructible<T>))
            && detail::evaluates_to<U, T>
  constexpr explicit variable(U&& u) noexcept(
    noexcept(detail::evaluate_for_node(std::declval<U>()))
    && std::is_nothrow_constructible_v<T, decltype(detail::evaluate_for_node(std::declval<U>()))>) :
    m_t(detail::evaluate_for_node(std::forward<U>(u)))
  { }

  variable(variable const&) = delete;
  variable(variable&&)      = delete;

  variable& operator=(variable const&) = delete;
  variable& operator=(variable&&)      = delete;

  /**
   * @brief Assigns a value to the variable, at once.
   *
   * The result is not a @c deferred expression, so an assignment that has already
   * happened cannot be mistaken for one that happens later: <tt>if_(c).then_(v = 0)</tt>
   * does not compile. To assign when an expression is evaluated, assign a @c deferred
   * expression: <tt>v = constant(0)</tt>.
   *
   * @param t Value to store.
   * @return A tag that no expression accepts.
   */
  constexpr detail::immediate_assignment
  operator=(T const& t) noexcept(std::is_nothrow_copy_assignable_v<T>)
  {
    m_t = t;
    return {};
  }

  /// @copydoc variable::operator=(T const&)
  constexpr detail::immediate_assignment
  operator=(T&& t) noexcept(std::is_nothrow_move_assignable_v<T>)
  {
    m_t = std::move(t);
    return {};
  }

  /**
   * @brief Builds a deferred assignment of @p e to the variable.
   *
   * Assigning a value stores it immediately; assigning a @c deferred expression
   * instead *builds* an expression that performs the assignment when it is evaluated,
   * which is what lets a loop clause, a body or a branch assign:
   * @code
   * n = 10;                                        // assigns now
   * auto ex = for_(i = constant(0), i < 10, ++i);  // assigns when ex is evaluated
   * @endcode
   *
   * The expression refers to the variable, so it must not outlive it.
   *
   * @tparam E Type of the assigned expression.
   * @param e Expression whose value is assigned to the variable.
   * @return An expression assigning to the variable when evaluated.
   */
  template<deferred_expression E>
  [[nodiscard]] constexpr auto
  operator=(E&& e) & noexcept(detail::assign_is_nothrow<detail::assign_to, variable&, E>())
  {
    return detail::assign_expression_t<detail::assign_to, variable&, E>(*this, std::forward<E>(e));
  }

  /// @brief Returns the stored value.
  [[nodiscard]] constexpr T const& operator()() const& noexcept
  {
    return m_t;
  }

  /// @brief Returns the stored value, as a mutable reference.
  [[nodiscard]] constexpr T& operator()() & noexcept
  {
    return m_t;
  }

  /// @brief Returns the stored value, moved out of the variable.
  [[nodiscard]] constexpr T operator()() && noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    return std::move(m_t);
  }

  /**
   * @brief Returns the stored value.
   *
   * A named spelling of <tt>operator()</tt>, for code that reads or writes the value
   * of the variable rather than evaluating it as an expression.
   */
  [[nodiscard]] constexpr T const& get() const& noexcept
  {
    return (*this)();
  }

  /**
   * @brief Returns the stored value, as a mutable reference.
   *
   * Writing through the reference changes the variable, as <tt>operator=</tt> with a
   * value does.
   */
  [[nodiscard]] constexpr T& get() & noexcept
  {
    return (*this)();
  }

  /// @brief Returns the stored value, moved out of the variable.
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
   * @brief Visits the variable with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
    requires expression_visitor<Visitor, variable>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, variable, subexpression_types>)
  {
    v(*this, nesting);
  }
};

/**
 * @brief Deduces the type of a variable from the value it is constructed with.
 *
 * The stored type is the decayed type of the value, so an array is held as a pointer
 * and a reference is never retained. A callable is evaluated first, as the constructor
 * does. A variable with no initial value names its type: <tt>variable<int> v;</tt>.
 *
 * @tparam T Type of the value to deduce.
 */
template<typename T>
variable(T&&) -> variable<detail::node_result_t<T&&>>;

} // namespace deferred

#endif
