// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <type_traits>

#include "deferred/conditional.hpp"
#include "deferred/detail/no_match_is_nothrow.hpp"

namespace {

struct throwing_from_nullopt
{
  throwing_from_nullopt(std::nullopt_t) noexcept(false);
};

} // namespace

TEST_CASE("no_match_is_nothrow_v holds for a void result", "[no-match-is-nothrow]")
{
  // A void expression reports nothing, so the path cannot throw.
  STATIC_CHECK(deferred::detail::no_match_is_nothrow_v<void>);
}

TEST_CASE("no_match_is_nothrow_v follows optional's nullopt constructor", "[no-match-is-nothrow]")
{
  STATIC_CHECK(deferred::detail::no_match_is_nothrow_v<std::optional<int>>);
  STATIC_CHECK(deferred::detail::no_match_is_nothrow_v<std::optional<std::optional<int>>>);
}

TEST_CASE("no_match_is_nothrow_v tracks a throwing construction", "[no-match-is-nothrow]")
{
  STATIC_CHECK(!deferred::detail::no_match_is_nothrow_v<throwing_from_nullopt>);
}

TEST_CASE("no_match_is_nothrow_v agrees with a non-finalized conditional", "[no-match-is-nothrow]")
{
  auto ex = deferred::if_(true).then_(1);

  STATIC_CHECK(noexcept(ex()) == deferred::detail::no_match_is_nothrow_v<decltype(ex())>);
}
