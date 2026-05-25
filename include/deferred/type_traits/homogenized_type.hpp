// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_TYPE_TRAITS_HOMOGENIZED_TYPE_HPP
#define DEFERRED_TYPE_TRAITS_HOMOGENIZED_TYPE_HPP

#include <tuple>
#include <type_traits>
#include <variant>

namespace deferred {

namespace detail {

/**
 * @brief Maps @c void to @c std::monostate for use in @c std::variant.
 * @tparam T Type to map.
 */
template<typename T>
using map_void_t = std::conditional_t<std::is_void_v<T>, std::monostate, T>;

/**
 * @brief Metafunction to create a unique list of types.
 * @tparam Ts Input types.
 */
template<typename... Ts>
struct unique_list
{
  using type = std::tuple<>;
};

/**
 * @brief Recursive case for @ref unique_list.
 * @tparam T Current type.
 * @tparam Ts Remaining types.
 */
template<typename T, typename... Ts>
struct unique_list<T, Ts...>
{
  /**
   * @brief Tail of the unique list.
   */
  using tail = typename unique_list<Ts...>::type;

  /**
   * @brief Resulting type list after potentially prepending @p T.
   */
  using type = std::conditional_t<(std::is_same_v<T, Ts> || ...),
                                  tail,
                                  decltype(std::tuple_cat(std::declval<std::tuple<T>>(),
                                                          std::declval<tail>()))>;
};

/**
 * @brief Deduce the result type of @p Ts...
 *
 * - If all @p Ts... are the same, return that type.
 * - Otherwise, return a @c std::variant of the unique types in @p Ts...,
 *   mapping @c void to @c std::monostate.
 *
 * @tparam Ts Types to deduce.
 * @return A @c std::type_identity containing the deduced result type.
 */
template<typename... Ts>
consteval auto deduce_homogenized_type()
{
  using unique_mapped = typename unique_list<map_void_t<Ts>...>::type;

  if constexpr (std::tuple_size_v<unique_mapped> == 1)
  {
    return std::type_identity<std::tuple_element_t<0, unique_mapped>>{};
  }
  else
  {
    return []<typename... Us>(std::tuple<Us...>*) {
      return std::type_identity<std::variant<Us...>>{};
    }((unique_mapped*)nullptr);
  }
}

/**
 * @brief Specialization for no types.
 */
template<>
consteval auto deduce_homogenized_type<>()
{
  return std::type_identity<void>{};
}

} // namespace detail

/**
 * @brief Metafunction to deduce the homogenized type of @p Ts..., or a variant if they are not all
 * identical.
 * @tparam Ts Types to deduce.
 *
 * - If all @p Ts... are identical, that type is used.
 * - Otherwise, a @c std::variant of the unique types in @p Ts... is used,
 *   mapping @c void to @c std::monostate.
 */
template<typename... Ts>
struct homogenized_type
{
  using deduced_type = typename decltype(detail::deduce_homogenized_type<Ts...>())::type;
  using type = std::conditional_t<std::is_same_v<deduced_type, std::monostate>, void, deduced_type>;
};

/**
 * @brief Alias for the deduced homogenized type.
 * @tparam Ts Types to deduce.
 */
template<typename... Ts>
using homogenized_type_t = typename homogenized_type<Ts...>::type;

} // namespace deferred

#endif
