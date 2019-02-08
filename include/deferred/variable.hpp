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

#include "type_traits.hpp"

namespace deferred
{

/// Holds a variable value.
template<typename T>
class [[nodiscard]] variable_
{
  T m_t{};

public:
  variable_() = default;

  constexpr explicit variable_(T const& t)
    : m_t(t)
  {}

  constexpr explicit variable_(T&& t) noexcept
    : m_t(std::move(t))
  {}

  variable_(variable_ const&) = delete;
  variable_(variable_&&) = delete;

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

  constexpr T& operator()() noexcept
  {
    return m_t;
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
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

/// Creates a new @ref variable_ that is initialized with @p t.
template<typename T>
constexpr auto variable(T&& t)
{
  return variable_<std::decay_t<T>>(std::forward<T>(t));
}

/// @brief Checks if @p T is a @ref variable_.
template<typename...>
struct is_variable
  : public std::false_type
{};

template<typename T>
struct is_variable<variable_<T>>
  : public std::true_type
{};

/// Alias for @c is_variable::type.
template<typename T>
using is_variable_t = typename is_variable<T>::type;

/// Alias for @c is_variable::value.
template<typename T>
inline constexpr bool is_variable_v = is_variable<T>::value;

template<typename T>
struct is_deferred<variable_<T>>
  : public std::true_type
{};

} // namespace deferred

#endif
