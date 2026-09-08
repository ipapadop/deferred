// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>

#include "deferred/constant.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"
#include "deferred/variable.hpp"
#include "deferred/while.hpp"

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

TEST_CASE("while with lambda", "[while-lambda]")
{
  auto i  = 0;
  auto j  = 0;
  auto ex = deferred::while_([&i] { return ++i < 10; }).do_([&j] { j += 2; });

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  ex();
  CHECK(i == 10);
  CHECK(j == 18);
}

TEST_CASE("while with constexpr", "[while-constexpr]")
{
  auto ex = deferred::while_([]() noexcept { return false; }).do_([]() noexcept { });

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(noexcept(ex()));
  ex();
}

TEST_CASE("while does not run its body when the condition starts false", "[while-no-iteration]")
{
  auto ran = 0;
  auto ex  = deferred::while_(false).do_([&ran] { ++ran; });

  ex();
  CHECK(ran == 0);
}

TEST_CASE("while builder reused gives independent loops", "[while-builders]")
{
  auto n    = deferred::variable<int>();
  auto a    = 0;
  auto b    = 0;
  auto step = [&n] {
    n = n() - 1;
  };

  auto const builder = deferred::while_([&n] { return n() != 0; });
  auto first         = builder.do_([&] {
    ++a;
    step();
  });
  auto second        = builder.do_([&] {
    ++b;
    step();
  });

  n = 3;
  first();
  CHECK(a == 3);
  CHECK(b == 0);

  n = 2;
  second();
  CHECK(a == 3);
  CHECK(b == 2);
}

TEST_CASE("while with move-only expressions", "[while-move-only]")
{
  auto ran = 0;
  auto ex  = deferred::while_([p = std::make_unique<int>(3)]() mutable {
              return (*p)-- > 0;
             }).do_([&ran, p = std::make_unique<int>(0)] { ran += *p + 1; });

  ex();
  CHECK(ran == 3);
}

TEST_CASE("while builder is copy constructible from a mutable lvalue", "[while-builders]")
{
  auto ran     = 0;
  auto builder = deferred::while_([&ran] { return ran++ < 3; });

  // Direct-initialization must select the copy constructor, not the forwarding one.
  decltype(builder) copy(builder);
  copy.do_([] { })();

  CHECK(ran == 4);
}

TEST_CASE("while builders are not deferred expressions", "[while-builders]")
{
  static_assert(!deferred::Deferred<decltype(deferred::while_(false))>);
}

TEST_CASE("building a while loop carries its exception guarantee", "[while-noexcept]")
{
  STATIC_CHECK(noexcept(deferred::while_(false).do_(1)));
  STATIC_CHECK(!noexcept(deferred::while_(false).do_(deferred::constant(throwing_expression{}))));
}

TEST_CASE("while accounts for throwing condition conversion", "[while-noexcept]")
{
  auto ex = deferred::while_([]() noexcept { return throwing_boolean{}; }).do_([]() noexcept { });

  static_assert(!noexcept(ex()));
}
