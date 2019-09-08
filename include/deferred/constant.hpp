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

#include "evaluate.hpp"
#include "type_traits/is_constant.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

template<typename T>
class constant_;

namespace detail {

template<typename T>
struct is_deferred<constant_<T>> : public std::true_type {};

} // namespace detail

/// Stores a constant.
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
 * Creates a constant for use in @c deferred expressions.
 *
 * If @p t is a callable type, this function will force its evaluation through
 * <tt>t()</tt>. This applies even if @p t is a @c deferred expression.
 */
template<typename T>
constexpr auto constant(T&& t)
{
  using result_type =
    std::decay_t<decltype(recursive_evaluate(std::forward<T>(t)))>;
  return constant_<result_type>(recursive_evaluate(std::forward<T>(t)));
}

} // namespace deferred

#endif
