// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/type_traits/is_constant_expression.hpp"
#include "deferred/while.hpp"

namespace {

struct throwing_boolean
{
  explicit operator bool() const noexcept(false)
  {
    return false;
  }
};

} // namespace

TEST_CASE("while with lambda", "[while-lambda]")
{
  auto i  = 0;
  auto j  = 0;
  auto ex = deferred::while_([&i] { return ++i < 10; }, [&j] { j += 2; });

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  ex();
  CHECK(i == 10);
  CHECK(j == 18);
}

TEST_CASE("while with constexpr", "[while-constexpr]")
{
  auto ex = deferred::while_([]() noexcept { return false; }, []() noexcept { });

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(noexcept(ex()));
  ex();
}

TEST_CASE("while accounts for throwing condition conversion", "[while-noexcept]")
{
  auto ex = deferred::while_([]() noexcept { return throwing_boolean{}; }, []() noexcept { });

  static_assert(!noexcept(ex()));
}
