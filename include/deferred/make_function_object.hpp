/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_MAKE_FUNCTION_OBJECT_HPP
#define DEFERRED_MAKE_FUNCTION_OBJECT_HPP

#include <type_traits>

namespace deferred {

namespace detail {

// Wraps a function pointer
template<typename F>
struct fun_ptr_wrapper
{
  F m_f;

  constexpr fun_ptr_wrapper(F f) noexcept : m_f{f}
  { }

  template<typename... T>
  constexpr decltype(auto) operator()(T&&... t) const noexcept(noexcept(m_f(std::forward<T>(t)...)))
  {
    return m_f(std::forward<T>(t)...);
  }
};

} // namespace detail

/// Creates a function object from @p f.
template<typename F>
constexpr decltype(auto) make_function_object(F&& f) noexcept(noexcept(std::forward<F>(f)))
{
  if constexpr (std::is_function_v<std::remove_reference_t<F>>)
  {
    return detail::fun_ptr_wrapper<std::decay_t<F>>(std::forward<F>(f));
  }
  else
  {
    return std::forward<F>(f);
  }
}

} // namespace deferred

#endif
