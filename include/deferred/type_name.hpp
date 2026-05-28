// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_TYPE_NAME_HPP
#define DEFERRED_TYPE_NAME_HPP

#include <string>
#include <string_view>
#include <type_traits>
#include <typeinfo>

#ifdef __GNUG__
#  include <cstdlib>
#  include <memory>

#  include <cxxabi.h>
#endif

namespace deferred {

/**
 * @brief Returns the unmangled name of @p T as a @c std::string.
 *
 * This solution initially appeared in
 * https://stackoverflow.com/questions/18369128/how-can-i-see-the-type-deduced-for-a-template-type-parameter/18369732#18369732
 */
template<typename T>
[[nodiscard]] std::string type_name()
{
  using namespace std::literals;

  using TR = std::remove_reference_t<T>;

#ifdef __GNUG__
  std::unique_ptr<char, void (*)(void*)> own(
    abi::__cxa_demangle(typeid(TR).name(), nullptr, nullptr, nullptr),
    std::free);
  std::string r = own != nullptr ? own.get() : typeid(TR).name();
#else
  std::string r = typeid(TR).name();
#endif

  static constexpr auto const_suffix    = " const"sv;
  static constexpr auto volatile_suffix = " volatile"sv;
  static constexpr auto lref_suffix     = "&"sv;
  static constexpr auto rref_suffix     = "&&"sv;

  std::size_t extra_size = 0;
  if constexpr (std::is_const_v<TR>)
  {
    extra_size += const_suffix.size();
  }
  if constexpr (std::is_volatile_v<TR>)
  {
    extra_size += volatile_suffix.size();
  }
  if constexpr (std::is_lvalue_reference_v<T>)
  {
    extra_size += lref_suffix.size();
  }
  else if constexpr (std::is_rvalue_reference_v<T>)
  {
    extra_size += rref_suffix.size();
  }

  if (extra_size > 0)
  {
    r.reserve(r.size() + extra_size);
    if constexpr (std::is_const_v<TR>)
    {
      r += const_suffix;
    }
    if constexpr (std::is_volatile_v<TR>)
    {
      r += volatile_suffix;
    }
    if constexpr (std::is_lvalue_reference_v<T>)
    {
      r += lref_suffix;
    }
    else if constexpr (std::is_rvalue_reference_v<T>)
    {
      r += rref_suffix;
    }
  }

  return r;
}

/// @copydoc type_name()
template<typename T>
[[nodiscard]] auto type_name(T&& t)
{
  return type_name<decltype(t)>();
}

} // namespace deferred

#endif
