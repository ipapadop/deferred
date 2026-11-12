// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "deferred/detail/immediate_assignment.hpp"

TEST_CASE("immediate_assignment carries nothing and cannot be evaluated",
          "[detail-immediate-assignment]")
{
  using deferred::detail::immediate_assignment;

  static_assert(std::is_empty_v<immediate_assignment>);
  static_assert(!std::is_invocable_v<immediate_assignment>);
  static_assert(std::is_trivially_copyable_v<immediate_assignment>);
}
