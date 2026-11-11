// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include "deferred/constant.hpp"
#include "deferred/seq.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"
#include "deferred/variable.hpp"

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

TEST_CASE("seq accounts for copying its last result", "[seq]")
{
  auto s  = deferred::variable(std::string("x"));
  auto n  = deferred::variable(1);
  auto ex = deferred::seq(deferred::constant(0), s);

  // The last result is returned by value, and copying a std::string can throw.
  static_assert(std::is_same_v<decltype(ex()), std::string>);
  static_assert(!noexcept(ex()));
  // An element evaluated only for effect is not copied.
  static_assert(noexcept(deferred::seq(s, n)()));

  CHECK(ex() == "x");
}

TEST_CASE("a seq can be copied from a non-const lvalue", "[seq]")
{
  auto one = deferred::seq(deferred::constant(1));
  auto two = deferred::seq(deferred::constant(1), deferred::constant(2));

  // Direct-initialization considers the explicit forwarding constructor, which must
  // not be chosen over the copy constructor.
  auto one_copy(one);
  auto two_copy(two);

  static_assert(std::is_same_v<decltype(one_copy), decltype(one)>);
  CHECK(one_copy() == 1);
  CHECK(two_copy() == 2);
}
