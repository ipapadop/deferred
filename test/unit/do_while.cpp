// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "deferred/constant.hpp"
#include "deferred/do_while.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"
#include "deferred/variable.hpp"

namespace {

struct throwing_expression
{
  throwing_expression() = default;
  throwing_expression(throwing_expression const&) noexcept(false)
  { }

  throwing_expression(throwing_expression&&) noexcept(false)
  { }
};

struct throwing_boolean
{
  explicit operator bool() const noexcept(false)
  {
    return false;
  }
};

} // namespace

TEST_CASE("do-while with lambda", "[do-while-lambda]")
{
  auto i  = 0;
  auto j  = 0;
  auto ex = deferred::do_([&j] { j += 2; }).while_([&i] { return ++i < 10; });

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  ex();
  CHECK(i == 10);
  CHECK(j == 20);
}

TEST_CASE("do-while with constexpr", "[do-while-constexpr]")
{
  auto ex = deferred::do_([]() noexcept { }).while_([]() noexcept { return false; });

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(noexcept(ex()));
  ex();
}

TEST_CASE("do-while runs its body once when the condition starts false", "[do-while-one-iteration]")
{
  auto ran = 0;
  auto ex  = deferred::do_([&ran] { ++ran; }).while_(false);

  ex();
  CHECK(ran == 1);
}

TEST_CASE("do-while builder reused gives independent loops", "[do-while-builders]")
{
  auto n    = deferred::variable<int>();
  auto runs = 0;
  auto a    = 0;
  auto b    = 0;

  auto const builder = deferred::do_([&] {
    ++runs;
    n = n() - 1;
  });
  auto first         = builder.while_([&] {
    ++a;
    return n() != 0;
  });
  auto second        = builder.while_([&] {
    ++b;
    return false;
  });

  n = 3;
  first();
  CHECK(runs == 3);
  CHECK(a == 3);
  CHECK(b == 0);

  n = 3;
  second();
  CHECK(runs == 4);
  CHECK(n() == 2);
  CHECK(a == 3);
  CHECK(b == 1);

  // Building the second loop must not have consumed the shared body.
  n = 1;
  first();
  CHECK(runs == 5);
  CHECK(n() == 0);
}

TEST_CASE("do-while with move-only expressions", "[do-while-move-only]")
{
  auto ran = 0;
  auto ex  = deferred::do_([&ran, p = std::make_unique<int>(0)] {
              ran += *p + 1;
             }).while_([p = std::make_unique<int>(3)]() mutable { return --(*p) > 0; });

  ex();
  CHECK(ran == 3);
}

TEST_CASE("do-while builder is copy constructible from a mutable lvalue", "[do-while-builders]")
{
  auto ran     = 0;
  auto builder = deferred::do_([&ran] { ++ran; });

  // Direct-initialization must select the copy constructor, not the forwarding one.
  decltype(builder) copy(builder);
  copy.while_(false)();

  CHECK(ran == 1);
}

TEST_CASE("do-while builders are not deferred expressions", "[do-while-builders]")
{
  static_assert(!deferred::Deferred<decltype(deferred::do_(1))>);
}

TEST_CASE("building a do-while loop carries its exception guarantee", "[do-while-noexcept]")
{
  STATIC_CHECK(noexcept(deferred::do_(1).while_(false)));
  STATIC_CHECK(!noexcept(deferred::do_(1).while_(deferred::constant(throwing_expression{}))));
}

TEST_CASE("do-while accounts for throwing condition conversion", "[do-while-noexcept]")
{
  auto ex = deferred::do_([]() noexcept { }).while_([]() noexcept { return throwing_boolean{}; });

  static_assert(!noexcept(ex()));
}
