// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <type_traits>

#include "deferred/conditional.hpp"
#include "deferred/detail/unmatched_result.hpp"
#include "deferred/switch.hpp"

TEST_CASE("unmatched_result_t passes a finalized result through", "[unmatched-result]")
{
  STATIC_CHECK(std::is_same_v<deferred::detail::unmatched_result_t<true, int>, int>);
  STATIC_CHECK(std::is_same_v<deferred::detail::unmatched_result_t<true, void>, void>);
  STATIC_CHECK(
    std::is_same_v<deferred::detail::unmatched_result_t<true, std::string>, std::string>);
}

TEST_CASE("unmatched_result_t wraps a non-finalized result in optional", "[unmatched-result]")
{
  STATIC_CHECK(std::is_same_v<deferred::detail::unmatched_result_t<false, int>, std::optional<int>>);
  STATIC_CHECK(std::is_same_v<deferred::detail::unmatched_result_t<false, std::string>,
                              std::optional<std::string>>);
}

TEST_CASE("unmatched_result_t leaves a void result alone", "[unmatched-result]")
{
  // Nothing to report when the branches produce no value.
  STATIC_CHECK(std::is_same_v<deferred::detail::unmatched_result_t<false, void>, void>);
}

TEST_CASE("unmatched_result_t agrees with the expressions that use it", "[unmatched-result]")
{
  auto finalized     = deferred::if_(true).then_(1).else_(2);
  auto not_finalized = deferred::if_(true).then_(1);
  auto switched      = deferred::switch_(1).case_(1).then_(2);

  STATIC_CHECK(std::is_same_v<decltype(finalized()),
                              deferred::detail::unmatched_result_t<true, int>>);
  STATIC_CHECK(std::is_same_v<decltype(not_finalized()),
                              deferred::detail::unmatched_result_t<false, int>>);
  STATIC_CHECK(
    std::is_same_v<decltype(switched()), deferred::detail::unmatched_result_t<false, int>>);
}
