// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <tuple>
#include <type_traits>

#include "deferred/detail/tuple_references.hpp"

TEST_CASE("tuple_references of an empty tuple is empty", "[tuple-references]")
{
  static_assert(
    std::is_same_v<decltype(deferred::detail::tuple_references(std::tuple<>{})), std::tuple<>>);
}

TEST_CASE("tuple_references refers to the elements in order", "[tuple-references]")
{
  auto const t    = std::tuple<int, char>(1, 'a');
  auto const refs = deferred::detail::tuple_references(t);

  static_assert(std::is_same_v<decltype(refs), std::tuple<int const&, char const&> const>);
  CHECK(&std::get<0>(refs) == &std::get<0>(t));
  CHECK(&std::get<1>(refs) == &std::get<1>(t));
}

TEST_CASE("tuple_references keeps a reference element as it is", "[tuple-references]")
{
  int i        = 1;
  auto const t = std::tuple<int&, int>(i, 2);

  static_assert(
    std::is_same_v<decltype(deferred::detail::tuple_references(t)), std::tuple<int&, int const&>>);
  CHECK(&std::get<0>(deferred::detail::tuple_references(t)) == &i);
}

TEST_CASE("tuple_references is noexcept and usable in a constant expression", "[tuple-references]")
{
  static constexpr auto t = std::tuple<int, int>(3, 4);

  static_assert(noexcept(deferred::detail::tuple_references(t)));
  static_assert(std::get<1>(deferred::detail::tuple_references(t)) == 4);
}
