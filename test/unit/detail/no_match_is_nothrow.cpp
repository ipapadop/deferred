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

TEST_CASE("no_match_is_nothrow_v is one half of a non-finalized conditional's guarantee",
          "[no-match-is-nothrow]")
{
  auto ex = deferred::if_(true).then_(1);

  // The trait covers the path where nothing matched. Evaluating also wraps a matched
  // branch in the optional, which is only nothrow where the standard library says it
  // is: optional's converting constructor has no noexcept specification, and libc++
  // does not add one.
  STATIC_CHECK(noexcept(ex())
               == (deferred::detail::no_match_is_nothrow_v<decltype(ex())>
                   && std::is_nothrow_constructible_v<std::optional<int>, int>));
}
