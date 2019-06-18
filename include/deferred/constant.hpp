/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_CONSTANT_HPP
#define DEFERRED_CONSTANT_HPP

#include <type_traits>
#include <utility>

#include "type_traits/is_constant_expression.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

/// Holds a constant value.
template<typename T>
class constant_
{
public:
  using constant_expression = std::true_type;

private:
  T m_t;

public:
  /// Constructs a constant_ from @p u.
  template<typename U, std::enable_if_t<std::is_convertible_v<U, T>>* = nullptr>
  constexpr explicit constant_(U&& u) : m_t(std::forward<U>(u))
  {}

  constant_(constant_ const&) = default;
  constant_(constant_&&)      = default;

  constant_& operator=(constant_ const&) = delete;
  constant_& operator=(constant_&&) = delete;

  /// Returns the stored value.
  constexpr T const& operator()() const& noexcept
  {
    return m_t;
  }

  /// @copydoc operator()()
  constexpr T operator()() && noexcept
  {
    return std::move(m_t);
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/**
 * Creates a new @ref constant_ that is initialized from a constant expression.
 *
 * @warning This function will force <tt>ex()</tt>.
 */
template<typename Expression,
         std::enable_if_t<
           is_deferred_v<Expression> && is_constant_expression_v<Expression>>* =
           nullptr>
constexpr auto constant(Expression&& ex)
{
  using internal_type = std::decay_t<decltype(std::forward<Expression>(ex)())>;
  return constant_<internal_type>(std::forward<Expression>(ex)());
}

/// Creates a new @ref variable_ that is initialized with @p t.
template<
  typename T,
  std::enable_if_t<!is_deferred_v<T> && !std::is_invocable_v<T>>* = nullptr>
constexpr auto constant(T&& t)
{
  using internal_type = std::decay_t<T>;
  return constant_<internal_type>(std::forward<T>(t));
}

/**
 * Creates a new @ref constant_ that is initialized from a callable @p f.
 *
 * @warning This function will force <tt>f()</tt>.
 */
template<typename F,
         std::enable_if_t<!is_deferred_v<F> && std::is_invocable_v<F>>* = nullptr>
constexpr auto constant(F&& f)
{
  using internal_type = decltype(std::forward<F>(f)());
  return constant_<internal_type>(std::forward<F>(f)());
}

} // namespace deferred

#endif
