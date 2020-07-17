/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_TYPE_NAME_HPP
#define DEFERRED_TYPE_NAME_HPP

#include <string>
#include <type_traits>
#include <typeinfo>

#ifdef __GNUG__
#  include <cstdlib>
#  include <memory>

#  include <cxxabi.h>
#endif

namespace deferred {

/**
 * Returns the unmangled name of @p T as a @c std::string.
 *
 * This solution initially appeared in
 * https://stackoverflow.com/questions/18369128/how-can-i-see-the-type-deduced-for-a-template-type-parameter/18369732#18369732
 */
template<typename T>
std::string type_name()
{
  using TR = std::remove_reference_t<T>;

#ifdef __GNUG__
  std::unique_ptr<char, void (*)(void*)> own(
    abi::__cxa_demangle(typeid(TR).name(), nullptr, nullptr, nullptr),
    std::free);
  std::string r = own != nullptr ? own.get() : typeid(TR).name();
#else
  std::string r = typeid(TR).name();
#endif

  if (std::is_const<TR>::value)
  {
    r += " const";
  }
  if (std::is_volatile<TR>::value)
  {
    r += " volatile";
  }
  if (std::is_lvalue_reference<T>::value)
  {
    r += "&";
  }
  else if (std::is_rvalue_reference<T>::value)
  {
    r += "&&";
  }
  return r;
}

/// @copydoc type_name()
template<typename T>
auto type_name(T&& t)
{
  return type_name<decltype(t)>();
}

} // namespace deferred

#endif
