// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <optional>

#include "deferred/switch.hpp"
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

struct comparable
{ };

throwing_boolean operator==(comparable const&, comparable const&) noexcept
{
  return {};
}

} // namespace

TEST_CASE("switch with only a default", "[switch-only-default]")
{
  auto ex = deferred::switch_(1).default_(2);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 2);
}

TEST_CASE("switch with a default lambda", "[switch-default-lambda]")
{
  auto ex = deferred::switch_(1).default_([] { return 2; });
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 2);
}

TEST_CASE("switch with a literal case", "[switch-case-literal]")
{
  auto ex = deferred::switch_(2).case_(2).then_(0).default_(100);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 0);

  auto other = deferred::switch_(1).case_(2).then_(0).default_(100);
  CHECK(other() == 100);
}

TEST_CASE("switch with a lambda case", "[switch-case-lambda]")
{
  auto ex = deferred::switch_(2).case_([] { return 2; }).then_([] { return 0; }).default_(100);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 0);
}

TEST_CASE("switch without a default", "[switch-no-default]")
{
  auto ex  = deferred::switch_(2).case_(1).then_(10).case_(2).then_(20);
  auto res = ex();
  static_assert(std::is_same_v<decltype(res), std::optional<int>>);
  CHECK(res.has_value());
  CHECK(*res == 20);

  auto unmatched = deferred::switch_(3).case_(1).then_(10).case_(2).then_(20);
  CHECK(!unmatched().has_value());
}

TEST_CASE("switch with two literals", "[switch-two-literals]")
{
  auto ex = deferred::switch_(2)
              .case_(1)
              .then_([] { return 0; })
              .case_(2)
              .then_([] { return 10; })
              .default_(100);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(!noexcept(ex()));
  CHECK(ex() == 10);

  constexpr auto no_throw_ex = deferred::switch_(2).case_(2).then_(10).default_(100);
  static_assert(noexcept(no_throw_ex()));
}

TEST_CASE("switch with three literals", "[switch-three-literals]")
{
  auto ex = deferred::switch_(3)
              .case_(1)
              .then_([] { return 0; })
              .case_(2)
              .then_([] { return 10; })
              .case_(3)
              .then_([] { return 100; })
              .default_(1000);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 100);
}

TEST_CASE("switch with two literals defaulting", "[switch-two-literals-default]")
{
  auto ex = deferred::switch_(10)
              .case_(1)
              .then_([] { return 0; })
              .case_(2)
              .then_([] { return 10; })
              .default_(100);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 100);
}

TEST_CASE("switch with three literals defaulting", "[switch-three-literals-default]")
{
  auto ex = deferred::switch_(0)
              .case_(1)
              .then_([] { return 0; })
              .case_(2)
              .then_([] { return 10; })
              .case_(3)
              .then_([] { return 100; })
              .default_(1000);
  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  CHECK(ex() == 1000);
}

TEST_CASE("switch with heterogeneous types", "[switch-variant]")
{
  auto ex = deferred::switch_(1).case_(1).then_(42).default_("unknown");

  using result_type = decltype(ex());
  static_assert(std::is_same_v<result_type, std::variant<char const*, int>>);

  auto res = ex();
  CHECK(std::get<int>(res) == 42);

  auto ex2  = deferred::switch_(2).case_(1).then_(42).default_("unknown");
  auto res2 = ex2();
  CHECK(std::get<char const*>(res2) == std::string("unknown"));
}

TEST_CASE("case appended with heterogeneous type", "[switch-append-variant]")
{
  auto ex       = deferred::switch_(2).case_(1).then_(42).default_("unknown");
  auto expanded = std::move(ex).case_(2).then_(2.5);

  using result_type = decltype(expanded());
  static_assert(std::is_same_v<result_type, std::variant<char const*, int, double>>);

  CHECK(std::get<double>(expanded()) == 2.5);
}

TEST_CASE("case appended to switch with move-only body", "[switch-append-move-only]")
{
  auto ex       = deferred::switch_(1)
                    .case_(1)
                    .then_([value = std::make_unique<int>(42)] { return *value; })
                    .default_(0);
  auto expanded = std::move(ex).case_(2).then_(2);

  CHECK(expanded() == 42);
}

TEST_CASE("case appended to switch with variable condition", "[switch-append-variable]")
{
  auto v = deferred::variable<int>();
  v      = 2;

  auto ex       = deferred::switch_(v).case_(1).then_(10).default_(0);
  auto expanded = std::move(ex).case_(2).then_(20);

  CHECK(expanded() == 20);

  v = 1;
  CHECK(expanded() == 10);
}

TEST_CASE("case appended with void result", "[switch-append-void]")
{
  auto ex       = deferred::switch_(2).case_(1).then_(1).default_(0);
  auto expanded = std::move(ex).case_(2).then_([] { });

  using result_type = decltype(expanded());
  static_assert(std::is_same_v<result_type, std::variant<int, std::monostate>>);

  CHECK(std::holds_alternative<std::monostate>(expanded()));
}

TEST_CASE("case appended after the default is checked before it", "[switch-append-order]")
{
  auto ex       = deferred::switch_(2).case_(1).then_(10).default_(0);
  auto expanded = ex.case_(2).then_(20);

  CHECK(ex() == 0);
  CHECK(expanded() == 20);
}

TEST_CASE("switch builders copy from const lvalues", "[switch-builders]")
{
  auto const ex      = deferred::switch_(2).case_(2).then_(20);
  auto const builder = ex.case_(3);

  CHECK(ex.default_(0)() == 20);
  CHECK(builder.then_(30)().value() == 20);
}

TEST_CASE("switch with no cases and no default", "[switch-empty]")
{
  auto ex = deferred::switch_(1);

  static_assert(deferred::is_constant_expression_v<decltype(ex)>);
  static_assert(std::is_void_v<decltype(ex())>);
  ex();
}

TEST_CASE("switch with void bodies", "[switch-void]")
{
  auto i  = 0;
  auto ex = deferred::switch_(1).case_(1).then_([&] { ++i; }).default_([&] { i += 10; });

  static_assert(std::is_void_v<decltype(ex())>);
  ex();
  CHECK(i == 1);

  auto ex2 = deferred::switch_(2).case_(1).then_([&] { ++i; }).default_([&] { i += 10; });
  ex2();
  CHECK(i == 11);
}

TEST_CASE("switch without a default and with void bodies", "[switch-void-no-default]")
{
  auto i  = 0;
  auto ex = deferred::switch_(2).case_(1).then_([&] { ++i; });

  static_assert(std::is_void_v<decltype(ex())>);
  ex();
  CHECK(i == 0);
}

TEST_CASE("switch evaluates the condition once and only the matched body", "[switch-short-circuit]")
{
  auto conditions = 0;
  auto bodies     = 0;
  auto condition  = [&conditions] {
    ++conditions;
    return 2;
  };

  auto ex = deferred::switch_(condition)
              .case_(1)
              .then_([&bodies] { return ++bodies; })
              .case_(2)
              .then_([&bodies] { return bodies += 10; })
              .default_([&bodies] { return bodies += 100; });

  CHECK(ex() == 10);
  CHECK(conditions == 1);
  CHECK(bodies == 10);
}

TEST_CASE("switch without a default with constexpr", "[switch-no-default-constexpr]")
{
  constexpr auto ex = deferred::switch_(2).case_(2).then_(10);

  // Wrapping the result in std::optional is only nothrow where the standard
  // library says it is: optional's converting constructor has no noexcept
  // specification, and libc++ does not add one.
  static_assert(noexcept(ex()) == std::is_nothrow_constructible_v<std::optional<int>, int>);
  static_assert(ex().has_value());
  static_assert(*ex() == 10);

  constexpr auto unmatched = deferred::switch_(3).case_(2).then_(10);
  static_assert(!unmatched().has_value());
}

TEST_CASE("switch builder keeps a variable label by reference", "[switch-builders]")
{
  auto label         = deferred::variable<int>();
  auto const builder = deferred::switch_(2).case_(label);
  auto ex            = builder.then_(20).default_(0);

  label = 2;
  CHECK(ex() == 20);

  label = 3;
  CHECK(ex() == 0);
}

TEST_CASE("switch case returning a deferred expression", "[switch-nested-result]")
{
  auto v = deferred::variable<int>();
  auto ex =
    deferred::switch_(1).case_(1).then_([&v] { return deferred::constant(v() + 1); }).default_(0);

  static_assert(std::is_same_v<decltype(ex()), int>);

  v = 41;
  CHECK(ex() == 42);
}

TEST_CASE("switch bodies holding variables", "[switch-variable-body]")
{
  auto a  = deferred::variable<int>();
  auto b  = deferred::variable<int>();
  auto ex = deferred::switch_(1).case_(1).then_(a).default_(b);

  static_assert(std::is_same_v<decltype(ex()), int>);

  a = 1;
  b = 2;
  CHECK(ex() == 1);
}

TEST_CASE("switch builder is copy constructible from a mutable lvalue", "[switch-builders]")
{
  auto builder = deferred::switch_(1).case_(1);
  decltype(builder) copy(builder);

  CHECK(copy.then_(2)().value() == 2);
}

TEST_CASE("switch builders are not deferred expressions", "[switch-builders]")
{
  static_assert(!deferred::Deferred<decltype(deferred::switch_(1).case_(1))>);
}

TEST_CASE("switch with void bodies can be noexcept", "[switch-noexcept]")
{
  auto i  = 0;
  auto ex = deferred::switch_(1).case_(1).then_([&i]() noexcept { ++i; }).default_([&i]() noexcept {
    ++i;
  });

  // A void result is never constructed, so nothing on that path can throw.
  static_assert(std::is_void_v<decltype(ex())>);
  static_assert(noexcept(ex()));

  auto without_default = deferred::switch_(1).case_(1).then_([&i]() noexcept { ++i; });
  static_assert(noexcept(without_default()));

  auto throwing_body = deferred::switch_(1).case_(1).then_([&i] { ++i; });
  static_assert(!noexcept(throwing_body()));
}

TEST_CASE("switch accounts for throwing comparison conversion", "[switch-noexcept]")
{
  auto ex = deferred::switch_(comparable{}).case_(comparable{}).then_(1).default_(0);

  static_assert(!noexcept(ex()));
}
