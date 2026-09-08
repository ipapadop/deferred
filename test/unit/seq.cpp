// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <type_traits>
#include <vector>

#include "deferred/constant.hpp"
#include "deferred/seq.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"

TEST_CASE("seq evaluates its expressions left to right", "[seq]")
{
  std::vector<int> order;
  auto ex = deferred::seq([&order] { order.push_back(1); },
                          [&order] { order.push_back(2); },
                          [&order] { order.push_back(3); });

  ex();
  CHECK(order == std::vector{1, 2, 3});
}

TEST_CASE("seq yields the result of its last expression", "[seq]")
{
  auto ex = deferred::seq(1, 2, 3);

  CHECK(ex() == 3);
}

TEST_CASE("seq of void expressions is void", "[seq]")
{
  auto ran = 0;
  auto ex  = deferred::seq([&ran] { ++ran; }, [&ran] { ran += 10; });

  static_assert(std::is_void_v<decltype(ex())>);
  ex();
  CHECK(ran == 11);
}

TEST_CASE("seq of a single expression yields that expression", "[seq]")
{
  auto ex = deferred::seq(7);

  CHECK(ex() == 7);
}

TEST_CASE("seq re-evaluates every expression", "[seq]")
{
  auto ran = 0;
  auto ex  = deferred::seq([&ran] { ++ran; }, [&ran] { ++ran; });

  ex();
  ex();
  CHECK(ran == 4);
}

TEST_CASE("seq with constexpr", "[seq]")
{
  auto ex = deferred::seq([]() noexcept { }, []() noexcept { });

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(noexcept(ex()));
  ex();
}


namespace {

template<typename... T>
concept sequenceable = requires(T... t) { deferred::seq(t...); };

} // namespace

TEST_CASE("seq requires at least one expression", "[seq]")
{
  static_assert(sequenceable<int>);
  static_assert(!sequenceable<>);
}
