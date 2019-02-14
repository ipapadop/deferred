/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TYPE_TRAITS_MAKE_FUNCTION_OBJECT_HPP
#define DEFERRED_TYPE_TRAITS_MAKE_FUNCTION_OBJECT_HPP

#include <type_traits>

namespace deferred
{
namespace detail
{

// Wraps a function pointer
template<typename F>
struct fun_ptr_wrapper
{
  F m_f;

  constexpr fun_ptr_wrapper(F f) noexcept
    : m_f{f}
  {}

  template<typename... T>
  constexpr decltype(auto) operator()(T&&... t) const
  {
    return m_f(std::forward<T>(t)...);
  }
};

} // namespace detail

/// Forms a function object from @p T if is a function pointer.
template<typename T>
using make_function_object_t =
  std::conditional_t<
    std::is_function_v<std::remove_reference_t<T>>,
    detail::fun_ptr_wrapper<std::decay_t<T>>,
    T>;

} // namespace deferred

#endif
