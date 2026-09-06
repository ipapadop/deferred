// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <tuple>
#include <type_traits>

#include "deferred/detail/unique_list.hpp"

namespace {

template<typename... Ts>
using unique_t = typename deferred::detail::unique_list<Ts...>::type;

} // namespace

TEST_CASE("unique_list of no types is an empty tuple", "[unique-list]")
{
  STATIC_CHECK(std::is_same_v<unique_t<>, std::tuple<>>);
}

TEST_CASE("unique_list of one type", "[unique-list]")
{
  STATIC_CHECK(std::is_same_v<unique_t<int>, std::tuple<int>>);
}

TEST_CASE("unique_list of already-distinct types preserves order", "[unique-list]")
{
  STATIC_CHECK(std::is_same_v<unique_t<int, double, char>, std::tuple<int, double, char>>);
}

TEST_CASE("unique_list collapses repeated types", "[unique-list]")
{
  STATIC_CHECK(std::is_same_v<unique_t<int, int>, std::tuple<int>>);
  STATIC_CHECK(std::is_same_v<unique_t<int, int, int>, std::tuple<int>>);
}

TEST_CASE("unique_list keeps the last occurrence of a duplicate", "[unique-list]")
{
  // A duplicated type is dropped at its earlier position, not its later one.
  STATIC_CHECK(std::is_same_v<unique_t<int, double, int>, std::tuple<double, int>>);
  STATIC_CHECK(std::is_same_v<unique_t<int, int, double>, std::tuple<int, double>>);
}

TEST_CASE("unique_list treats cv- and reference-qualified types as distinct", "[unique-list]")
{
  STATIC_CHECK(std::is_same_v<unique_t<int, int const>, std::tuple<int, int const>>);
  STATIC_CHECK(std::is_same_v<unique_t<int, int&>, std::tuple<int, int&>>);
  STATIC_CHECK(std::is_same_v<unique_t<int&, int&&>, std::tuple<int&, int&&>>);
}

TEST_CASE("unique_list does not map void", "[unique-list]")
{
  // Mapping void to monostate is the caller's job (see map_void_t).
  STATIC_CHECK(std::is_same_v<unique_t<void>, std::tuple<void>>);
  STATIC_CHECK(std::is_same_v<unique_t<void, void>, std::tuple<void>>);
}
