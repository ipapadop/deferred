// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <type_traits>

#include "deferred/constant.hpp"
#include "deferred/for.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"

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

TEST_CASE("for with lambda", "[for-lambda]")
{
  auto i   = 0;
  auto sum = 0;
  auto ex =
    deferred::for_([&i] { i = 0; }, [&i] { return i < 5; }, [&i] { ++i; }).do_([&] { sum += i; });

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  ex();
  CHECK(i == 5);
  CHECK(sum == 10);
}

TEST_CASE("for with constexpr", "[for-constexpr]")
{
  auto ex = deferred::for_([]() noexcept { }, []() noexcept { return false; }, []() noexcept { })
              .do_([]() noexcept { });

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(noexcept(ex()));
  ex();
}

TEST_CASE("for does not run its body when the condition starts false", "[for-no-iteration]")
{
  auto ran  = 0;
  auto step = 0;
  auto ex   = deferred::for_(0, false, [&step] { ++step; }).do_([&ran] { ++ran; });

  ex();
  CHECK(ran == 0);
  CHECK(step == 0);
}

TEST_CASE("for builder reused gives independent loops", "[for-builders]")
{
  auto n = 0;
  auto a = 0;
  auto b = 0;

  auto const builder = deferred::for_([&n] { n = 0; }, [&n] { return n < 3; }, [&n] { ++n; });
  auto first         = builder.do_([&a] { ++a; });
  auto second        = builder.do_([&b] { b += 2; });

  first();
  CHECK(a == 3);
  CHECK(b == 0);

  second();
  CHECK(a == 3);
  CHECK(b == 6);
}

TEST_CASE("for with move-only expressions", "[for-move-only]")
{
  auto n   = 0;
  auto ran = 0;
  auto ex  = deferred::for_([&n, p = std::make_unique<int>(0)] { n = *p; },
                            [&n] { return n < 3; },
                            [&n, p = std::make_unique<int>(1)] { n += *p; })
               .do_([&ran, p = std::make_unique<int>(1)] { ran += *p; });

  ex();
  CHECK(ran == 3);
}

TEST_CASE("for builders are not deferred expressions", "[for-builders]")
{
  static_assert(!deferred::Deferred<decltype(deferred::for_(0, false, 0))>);
}

TEST_CASE("building a for loop carries its exception guarantee", "[for-noexcept]")
{
  STATIC_CHECK(noexcept(deferred::for_(0, false, 0).do_(1)));
  STATIC_CHECK(
    !noexcept(deferred::for_(0, false, 0).do_(deferred::constant(throwing_expression{}))));
  STATIC_CHECK(
    !noexcept(deferred::for_(deferred::constant(throwing_expression{}), false, 0).do_(1)));
}

TEST_CASE("for accounts for throwing condition conversion", "[for-noexcept]")
{
  auto ex = deferred::for_([]() noexcept { },
                           []() noexcept { return throwing_boolean{}; },
                           []() noexcept { })
              .do_([]() noexcept { });

  static_assert(!noexcept(ex()));
}

TEST_CASE("a loop outlives the builder it was built from", "[for-builders]")
{
  auto n   = 0;
  auto ran = 0;

  auto make_loop = [&] {
    auto const builder = deferred::for_([&n] { n = 0; }, [&n] { return n < 3; }, [&n] { ++n; });
    // The clauses must be copied out of the builder, not referenced into it.
    return builder.do_([&ran] { ++ran; });
  };

  auto ex = make_loop();
  ex();
  CHECK(ran == 3);
}

TEST_CASE("a for loop evaluates to void", "[for-result]")
{
  auto ex = deferred::for_(0, false, 1).do_(1);

  static_assert(std::is_void_v<decltype(ex())>);
  ex();
}
