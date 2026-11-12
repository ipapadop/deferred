// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_VARIABLE_REF_HPP
#define DEFERRED_VARIABLE_REF_HPP

#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "type_traits/expression_visitor.hpp"

namespace deferred {

template<typename T>
class variable;

/**
 * @brief Refers to a @ref variable from inside an expression.
 *
 * An expression stores its operands by value. A variable is the exception in meaning,
 * not in storage: the expression must read and write the caller's variable, so it
 * stores a @c variable_ref to it. The variable must outlive every expression that
 * refers to it.
 *
 * A @c variable_ref is created when a variable is used in an expression; there is no
 * need to name it.
 *
 * @tparam T Type of the value of the variable; @c const when the variable is.
 */
template<typename T>
class variable_ref
{
public:
  using value_type = T;

  /// @brief Type of the variable referred to.
  using variable_type =
    std::conditional_t<std::is_const_v<T>, variable<std::remove_const_t<T>> const, variable<T>>;

  using subexpression_types = std::tuple<>;

  /// @brief The value of the variable can change between evaluations.
  static constexpr bool mutable_state = true;

private:
  variable_type* m_variable;

public:
  /**
   * @brief Constructs a reference to @p v.
   * @param v Variable to refer to.
   */
  constexpr explicit variable_ref(variable_type& v) noexcept : m_variable(std::addressof(v))
  { }

  /// @brief A temporary variable would not outlive the reference.
  variable_ref(variable_type&&) = delete;

  variable_ref(variable_ref const&) = default;

  variable_ref& operator=(variable_ref const&) = delete;

  /// @brief Returns the value of the variable.
  [[nodiscard]] constexpr T& operator()() const noexcept
  {
    return (*m_variable)();
  }

  /**
   * @brief Returns references to the subexpressions.
   * @return An empty @c std::tuple.
   */
  [[nodiscard]] constexpr auto subexpressions() const noexcept
  {
    return std::tuple<>{};
  }

  /**
   * @brief Visits the variable referred to with a visitor.
   *
   * The visitor is called with the variable, not with the reference, so a visitor
   * sees the same nodes whichever way an expression stores them.
   *
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
    requires expression_visitor<Visitor, variable_ref>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(noexcept(std::as_const(*m_variable).visit(v, nesting)))
  {
    std::as_const(*m_variable).visit(v, nesting);
  }
};

namespace detail {

// A visitor is called with the variable a variable_ref refers to.
template<typename Visitor, typename T>
struct visits_tree<Visitor, variable_ref<T>> :
  visits_tree<Visitor, variable<std::remove_const_t<T>>>
{ };

/**
 * @brief The @ref variable_ref an expression stores for an operand of type @p T.
 *
 * Has a member @c type when @p T is an lvalue reference to a @ref variable , const or
 * not, and none otherwise.
 *
 * @tparam T Operand type.
 */
template<typename T>
struct variable_ref_for
{ };

template<typename T>
struct variable_ref_for<variable<T>&> : std::type_identity<variable_ref<T>>
{ };

template<typename T>
struct variable_ref_for<variable<T> const&> : std::type_identity<variable_ref<T const>>
{ };

} // namespace detail

} // namespace deferred

#endif
