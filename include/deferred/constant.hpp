// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_CONSTANT_HPP
#define DEFERRED_CONSTANT_HPP

#include <type_traits>
#include <utility>

#include "evaluate.hpp"

namespace deferred {

/**
 * @brief Stores a constant value.
 * @tparam T Type of the constant.
 */
template<typename T>
class constant_
{
public:
  using value_type          = T;
  using subexpression_types = std::tuple<>;

private:
  [[no_unique_address]] T m_t;

public:
  /**
   * @brief Constructs a constant_ from @p u.
   * @tparam U Type of the value to initialize the constant with.
   * @param u Value to initialize the constant with.
   */
  template<std::convertible_to<T> U>
  constexpr explicit constant_(U&& u) : m_t(std::forward<U>(u))
  { }

  constant_(constant_ const&) = default;
  constant_(constant_&&)      = default;

  constant_& operator=(constant_ const&) = delete;
  constant_& operator=(constant_&&)      = delete;

  /// @brief Returns the stored value.
  [[nodiscard]] constexpr T const& operator()() const& noexcept
  {
    return m_t;
  }

  /// @copydoc operator()()
  [[nodiscard]] constexpr T operator()() && noexcept
  {
    return std::move(m_t);
  }

  /**
   * @brief Visits the constant with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
  }
};

/**
 * @brief Creates a constant for use in @c deferred expressions.
 *
 * If @p t is a callable type, this function will force its evaluation through
 * <tt>t()</tt>. This applies even if @p t is a @c deferred expression.
 *
 * @tparam T Type of the value to wrap.
 * @param t Value to wrap in a constant.
 * @return A constant_ object containing the evaluated value.
 */
template<typename T>
[[nodiscard]] constexpr auto constant(T&& t)
{
  using result_type = std::decay_t<decltype(recursive_evaluate(std::forward<T>(t)))>;
  return constant_<result_type>(recursive_evaluate(std::forward<T>(t)));
}

} // namespace deferred

#endif
