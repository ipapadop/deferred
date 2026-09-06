// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>
#include <variant>

#include "deferred/detail/deduce_homogenized_type.hpp"

namespace {

template<typename... Ts>
using deduced_t = typename decltype(deferred::detail::deduce_homogenized_type<Ts...>())::type;

} // namespace

TEST_CASE("deduce_homogenized_type of no types is void", "[deduce-homogenized-type]")
{
  STATIC_CHECK(std::is_void_v<deduced_t<>>);
}

TEST_CASE("deduce_homogenized_type collapses identical types", "[deduce-homogenized-type]")
{
  STATIC_CHECK(std::is_same_v<deduced_t<int>, int>);
  STATIC_CHECK(std::is_same_v<deduced_t<int, int>, int>);
  STATIC_CHECK(std::is_same_v<deduced_t<int, int, int>, int>);
}

TEST_CASE("deduce_homogenized_type falls back to a variant", "[deduce-homogenized-type]")
{
  STATIC_CHECK(std::is_same_v<deduced_t<int, double>, std::variant<int, double>>);
  STATIC_CHECK(std::is_same_v<deduced_t<int, std::string>, std::variant<int, std::string>>);
}

TEST_CASE("deduce_homogenized_type deduplicates before deciding", "[deduce-homogenized-type]")
{
  STATIC_CHECK(std::is_same_v<deduced_t<int, int, double>, std::variant<int, double>>);
  STATIC_CHECK(std::is_same_v<deduced_t<int, double, int, double>, std::variant<int, double>>);
}

TEST_CASE("deduce_homogenized_type maps void to monostate", "[deduce-homogenized-type]")
{
  // Unlike homogenized_type, this layer stops at monostate and does not map it back to void.
  STATIC_CHECK(std::is_same_v<deduced_t<void>, std::monostate>);
  STATIC_CHECK(std::is_same_v<deduced_t<void, void>, std::monostate>);
  STATIC_CHECK(std::is_same_v<deduced_t<int, void>, std::variant<int, std::monostate>>);
}

TEST_CASE("deduce_homogenized_type returns a type_identity", "[deduce-homogenized-type]")
{
  STATIC_CHECK(std::is_same_v<decltype(deferred::detail::deduce_homogenized_type<int>()),
                              std::type_identity<int>>);
}
