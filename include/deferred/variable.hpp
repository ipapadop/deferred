/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */
#ifndef DEFERRED_VARIABLE_HPP
#define DEFERRED_VARIABLE_HPP

#include <type_traits>
#include <utility>

#include "type_traits/is_constant_expression.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

/// Holds a variable value.
template<typename T>
class [[nodiscard]] variable_
{
public:
  using is_constant_expression = std::false_type;

private:
  T m_t{};

public:
  variable_() = default;

  constexpr explicit variable_(T const& t) : m_t(t)
  {}

  constexpr explicit variable_(T && t) noexcept : m_t(std::move(t))
  {}

  variable_(variable_ const&) = delete;
  variable_(variable_ &&)     = delete;

  variable_& operator=(variable_ const&) = delete;
  variable_& operator=(variable_&&) = delete;

  constexpr variable_& operator=(T const& t)
  {
    m_t = t;
    return *this;
  }

  constexpr variable_& operator=(T&& t) noexcept
  {
    m_t = std::move(t);
    return *this;
  }

  constexpr T& operator()() & noexcept
  {
    return m_t;
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor && v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/// Creates a new @ref variable_ that holds a @p T.
template<typename T>
constexpr variable_<T> variable() noexcept
{
  return {};
}

/**
 * Creates a new @ref variable_ that is initialized from a constant expression.
 *
 * @warning This function will force <tt>ex()</tt>.
 */
template<typename Expression,
         std::enable_if_t<
           is_deferred_v<Expression> && is_constant_expression_v<Expression>>* =
           nullptr>
constexpr auto variable(Expression&& ex)
{
  using internal_type = std::decay_t<decltype(std::forward<Expression>(ex)())>;
  return variable_<internal_type>(std::forward<Expression>(ex)());
}

/// Creates a new @ref variable_ that is initialized with @p t.
template<
  typename T,
  std::enable_if_t<!is_deferred_v<T> && !std::is_invocable_v<T>>* = nullptr>
constexpr auto variable(T&& t)
{
  using internal_type = std::decay_t<T>;
  return variable_<internal_type>(std::forward<T>(t));
}

/**
 * Creates a new @ref constant_ that is initialized from a callable @p f.
 *
 * @warning This function will force <tt>f()</tt>.
 */
template<typename F,
         std::enable_if_t<!is_deferred_v<F> && std::is_invocable_v<F>>* = nullptr>
constexpr auto variable(F&& f)
{
  using internal_type = decltype(std::forward<F>(f)());
  return variable_<internal_type>(std::forward<F>(f)());
}

} // namespace deferred

#endif
