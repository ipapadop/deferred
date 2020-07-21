/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */
#ifndef DEFERRED_VARIABLE_HPP
#define DEFERRED_VARIABLE_HPP

#include <type_traits>
#include <utility>

#include "evaluate.hpp"

namespace deferred {

/// Stores a variable value.
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

  constexpr explicit variable_(T const& t) : m_t(t)
  { }

  constexpr explicit variable_(T&& t) noexcept : m_t(std::move(t))
  { }

  variable_(variable_ const&) = delete;
  variable_(variable_&&)      = delete;

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

  constexpr T const& operator()() const& noexcept
  {
    return m_t;
  }

  constexpr T& operator()() & noexcept
  {
    return m_t;
  }

  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
  }
};

/// Creates a new @ref variable_ that holds a @p T.
template<typename T>
constexpr variable_<T> variable() noexcept
{
  return {};
}

/**
 * Creates a variable for use in @c deferred expressions.
 *
 * If @p t is a callable type, this function will force its evaluation through
 * <tt>t()</tt>. This applies even if @p t is a @c deferred expression.
 */
template<typename T>
constexpr auto variable(T&& t)
{
  using result_type = std::decay_t<decltype(recursive_evaluate(std::forward<T>(t)))>;
  return variable_<result_type>(recursive_evaluate(std::forward<T>(t)));
}

} // namespace deferred

#endif
