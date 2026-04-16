// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_VARIABLE_HPP
#define DEFERRED_VARIABLE_HPP

#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>

#include "evaluate.hpp"

namespace deferred {

/**
 * @brief Stores a variable value.
 * @tparam T The type of the variable to store.
 */
template<typename T>
class [[nodiscard]] variable_
{
public:
  using value_type          = T;
  using subexpression_types = void;

private:
  T m_t{};

public:
  variable_() = default;

  /**
   * @brief Constructs a variable_ with a copy of @p t.
   * @param t The value to initialize the variable with.
   */
  constexpr explicit variable_(T const& t) : m_t(t)
  { }

  /**
   * @brief Constructs a variable_ by moving @p t.
   * @param t The value to initialize the variable with.
   */
  constexpr explicit variable_(T&& t) noexcept : m_t(std::move(t))
  { }

  variable_(variable_ const&) = delete;
  variable_(variable_&&)      = delete;

  variable_& operator=(variable_ const&) = delete;
  variable_& operator=(variable_&&)      = delete;

  /**
   * @brief Assigns a value to the variable.
   */
  constexpr variable_& operator=(T const& t)
  {
    m_t = t;
    return *this;
  }

  /// @copydoc variable_::operator=(T const&)
  constexpr variable_& operator=(T&& t) noexcept
  {
    m_t = std::move(t);
    return *this;
  }

  /**
   * @brief Returns the stored value.
   */
  [[nodiscard]] constexpr T const& operator()() const& noexcept
  {
    return m_t;
  }

  /// @copydoc variable_::operator()() const& noexcept
  [[nodiscard]] constexpr T& operator()() & noexcept
  {
    return m_t;
  }

  /// @copydoc variable_::operator()() const& noexcept
  [[nodiscard]] constexpr T operator()() && noexcept
  {
    return std::move(m_t);
  }

  /**
   * @brief Visits the variable with a visitor.
   * @tparam Visitor The type of the visitor.
   * @param v The visitor.
   * @param nesting The nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
  }
};

/**
 * @brief Creates a new @ref variable_ that holds a default-initialized @p T.
 * @tparam T The type of the variable.
 * @return A newly constructed variable.
 */
template<typename T>
[[nodiscard]] constexpr auto variable() noexcept
{
  return variable_<T>{};
}

/**
 * Creates a variable for use in @c deferred expressions.
 *
 * If @p t is a callable type, this function will force its evaluation through
 * <tt>t()</tt>. This applies even if @p t is a @c deferred expression.
 *
 * @tparam T The type of the value to deduce.
 * @param t The value to wrap in a \ref variable_.
 * @return A variable representing the evaluated type of @p t.
 */
template<typename T>
[[nodiscard]] constexpr auto variable(T&& t)
{
  using result_type = std::decay_t<decltype(recursive_evaluate(std::forward<T>(t)))>;
  return variable_<result_type>(recursive_evaluate(std::forward<T>(t)));
}

} // namespace deferred

#endif
