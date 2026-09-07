// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

#include "deferred/conditional.hpp"
#include "deferred/constant.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"
#include "deferred/variable.hpp"

namespace {

struct throwing_boolean
{
  explicit operator bool() const noexcept(false)
  {
    return false;
  }
};

} // namespace

TEST_CASE("conditional with literal", "[conditional-literal]")
{
  auto ex = deferred::if_(true).then_(42).else_(10);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 42);
}

TEST_CASE("conditional with constexpr", "[conditional-constexpr]")
{
  constexpr auto ex = deferred::if_(true).then_(42).else_(10);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(ex() == 42, "constexpr failed");
  CHECK(ex() == 42);
}

TEST_CASE("conditional with different result types", "[conditional-diff-types]")
{
  auto ex = deferred::if_(true).then_(42).else_(false);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(std::get<int>(ex()) == 42);
}

TEST_CASE("conditional with lambda", "[conditional-lambdas]")
{
  auto i  = 0;
  auto j  = 0;
  auto k  = 0;
  auto ex = deferred::if_([&] { return i++ == 1; }).then_([&] { return ++j; }).else_([&] {
    return k += 2;
  });

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
  CHECK(ex() == 2);
  CHECK(i == 1);
  CHECK(j == 0);
  CHECK(k == 2);
}

TEST_CASE("conditional with mutable lambda", "[conditional-mutable-lambdas]")
{
  auto i  = 0;
  auto j  = 0;
  auto k  = 0;
  auto ex = deferred::if_([i]() mutable { return i++ == 1; })
              .then_([j]() mutable { return ++j; })
              .else_([k]() mutable { return k += 2; });

  static_assert(!deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
  CHECK(ex() == 2);
  CHECK(i == 0);
  CHECK(j == 0);
  CHECK(k == 0);
}

TEST_CASE("if_ with literal", "[if-literal]")
{
  auto ex_true  = deferred::if_(true).then_(42);
  auto res_true = ex_true();
  static_assert(std::is_same_v<decltype(res_true), std::optional<int>>);
  CHECK(res_true.has_value());
  CHECK(*res_true == 42);

  auto ex_false  = deferred::if_(false).then_(42);
  auto res_false = ex_false();
  CHECK(!res_false.has_value());
}

TEST_CASE("if_ with void return", "[if-void]")
{
  int i   = 0;
  auto ex = deferred::if_(true).then_([&] { ++i; });
  static_assert(std::is_void_v<decltype(ex())>);
  ex();
  CHECK(i == 1);

  auto ex2 = deferred::if_(false).then_([&] { ++i; });
  ex2();
  CHECK(i == 1);
}

TEST_CASE("if_ with constexpr", "[if-constexpr]")
{
  constexpr auto ex = deferred::if_(true).then_(42);
  // Wrapping the result in std::optional is only nothrow where the standard
  // library says it is: optional's converting constructor has no noexcept
  // specification, and libc++ does not add one.
  static_assert(noexcept(ex()) == std::is_nothrow_constructible_v<std::optional<int>, int>);
  static_assert(ex().has_value());
  static_assert(*ex() == 42);

  constexpr auto ex_false = deferred::if_(false).then_(42);
  static_assert(!ex_false().has_value());
}

TEST_CASE("if_ visit", "[if-visit]")
{
  auto ex   = deferred::if_(true).then_(42);
  int count = 0;
  ex.visit([&](auto const&, std::size_t) { ++count; });
  // 1 (if_expression) + 1 (branch) + 1 (condition) + 1 (then) = 4
  CHECK(count == 4);
}

TEST_CASE("conditional with else_if_", "[conditional-else-if]")
{
  auto ex = deferred::if_(false).then_(1).else_if_(true).then_(2).else_(3);
  CHECK(ex() == 2);

  auto ex2 = deferred::if_(false).then_(1).else_if_(false).then_(2).else_(3);
  CHECK(ex2() == 3);
}

TEST_CASE("conditional chains copy from const lvalues", "[conditional-chain-const]")
{
  auto const chain = deferred::if_(false).then_(1);

  CHECK(chain.else_if_(true).then_(2)().value() == 2);
  CHECK(chain.else_(3)() == 3);
}

TEST_CASE("conditional builders complete from const lvalues", "[conditional-builders]")
{
  auto const builder = deferred::if_(true);

  CHECK(builder.then_(2)().value() == 2);
  CHECK(builder.then_(3)().value() == 3);
}

TEST_CASE("conditional builder keeps a variable condition by reference", "[conditional-builders]")
{
  auto v             = deferred::variable<bool>();
  auto const builder = deferred::if_(v);
  auto ex            = builder.then_(42).else_(10);

  v = true;
  CHECK(ex() == 42);

  v = false;
  CHECK(ex() == 10);
}

TEST_CASE("conditional with mixed types (variant)", "[conditional-variant]")
{
  auto ex = deferred::if_(true).then_(42).else_("hello");

  using result_type = decltype(ex());
  static_assert(std::is_same_v<result_type, std::variant<int, char const*>>);

  auto res = ex();
  CHECK(std::get<int>(res) == 42);

  auto ex2  = deferred::if_(false).then_(42).else_("hello");
  auto res2 = ex2();
  CHECK(std::get<char const*>(res2) == std::string("hello"));
}

TEST_CASE("if_ with multiple else_if_ (optional)", "[if-else-if-optional]")
{
  auto ex  = deferred::if_(false).then_(1).else_if_(true).then_(2).else_if_(true).then_(3);
  auto res = ex();
  CHECK(res.has_value());
  CHECK(*res == 2);

  auto ex2  = deferred::if_(false).then_(1).else_if_(false).then_(2);
  auto res2 = ex2();
  CHECK(!res2.has_value());
}

TEST_CASE("conditional with void branches", "[conditional-void]")
{
  auto i  = 0;
  auto ex = deferred::if_(true).then_([&] { ++i; }).else_([&] { i += 10; });

  static_assert(std::is_void_v<decltype(ex())>);
  ex();
  CHECK(i == 1);

  auto ex2 = deferred::if_(false).then_([&] { ++i; }).else_([&] { i += 10; });
  ex2();
  CHECK(i == 11);
}

TEST_CASE("conditional mixing void and non-void branches", "[conditional-void-variant]")
{
  auto ex = deferred::if_(false).then_(42).else_([] { });

  using result_type = decltype(ex());
  static_assert(std::is_same_v<result_type, std::variant<int, std::monostate>>);
  CHECK(std::holds_alternative<std::monostate>(ex()));

  auto ex2 = deferred::if_(true).then_(42).else_([] { });
  CHECK(std::get<int>(ex2()) == 42);
}

TEST_CASE("conditional stops at the first matching branch", "[conditional-short-circuit]")
{
  auto checked = 0;
  auto taken   = 0;
  auto check   = [&checked] {
    ++checked;
    return true;
  };

  auto ex = deferred::if_(check)
              .then_([&taken] { return ++taken; })
              .else_if_(check)
              .then_([&taken] { return taken += 10; })
              .else_([&taken] { return taken += 100; });

  CHECK(ex() == 1);
  CHECK(checked == 1);
  CHECK(taken == 1);
}

TEST_CASE("conditional with move-only expressions", "[conditional-move-only]")
{
  auto ex = deferred::if_([p = std::make_unique<bool>(true)] { return *p; })
              .then_([p = std::make_unique<int>(1)] { return *p; })
              .else_([p = std::make_unique<int>(2)] { return *p; });

  CHECK(ex() == 1);
}

TEST_CASE("conditional branch returning a deferred expression", "[conditional-nested-result]")
{
  auto v  = deferred::variable<int>();
  auto ex = deferred::if_(true).then_([&v] { return deferred::constant(v() + 1); }).else_(0);

  // The branch yields a deferred expression; the result type is what evaluating it
  // produces, not the expression itself.
  static_assert(std::is_same_v<decltype(ex()), int>);

  v = 41;
  CHECK(ex() == 42);
}

TEST_CASE("conditional branches holding variables", "[conditional-variable-branch]")
{
  auto a  = deferred::variable<int>();
  auto b  = deferred::variable<int>();
  auto ex = deferred::if_(true).then_(a).else_(b);

  static_assert(std::is_same_v<decltype(ex()), int>);

  a = 1;
  b = 2;
  CHECK(ex() == 1);
}

TEST_CASE("conditional branches holding constants", "[conditional-constant-branch]")
{
  auto a  = deferred::constant(std::string("first"));
  auto b  = deferred::constant(std::string("second"));
  auto ex = deferred::if_(false).then_(a).else_(b);

  // The result is a value; returning a reference here would dangle.
  static_assert(std::is_same_v<decltype(ex()), std::string>);
  CHECK(ex() == "second");
}

TEST_CASE("conditional builder is copy constructible from a mutable lvalue",
          "[conditional-builders]")
{
  auto builder = deferred::if_(true);
  decltype(builder) copy(builder);

  CHECK(copy.then_(1)().value() == 1);
}

TEST_CASE("conditional builders are not deferred expressions", "[conditional-builders]")
{
  static_assert(!deferred::Deferred<decltype(deferred::if_(true))>);
  static_assert(!deferred::Deferred<decltype(deferred::if_(true).then_(1).else_if_(false))>);
}

TEST_CASE("conditional accounts for throwing condition conversion", "[conditional-noexcept]")
{
  auto ex = deferred::if_([]() noexcept { return throwing_boolean{}; }).then_(1).else_(2);

  static_assert(!noexcept(ex()));
}
