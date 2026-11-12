// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "deferred/constant.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"
#include "deferred/variable.hpp"

namespace {

struct throwing_value
{
  throwing_value() noexcept(false);
  throwing_value(throwing_value const&) noexcept(false);
  throwing_value(throwing_value&&) noexcept(false);
  throwing_value& operator=(throwing_value const&) noexcept(false);
  throwing_value& operator=(throwing_value&&) noexcept(false);
};

} // namespace

static_assert(!noexcept(deferred::variable<throwing_value>()));
static_assert(!noexcept(deferred::variable<throwing_value>(std::declval<throwing_value&&>())));
static_assert(!noexcept(
  std::declval<deferred::variable<throwing_value>&>() = std::declval<throwing_value&&>()));
static_assert(!noexcept(std::declval<deferred::variable<throwing_value>&&>()()));
static_assert(!noexcept(std::declval<deferred::variable<throwing_value>&&>().get()));

TEST_CASE("empty variable", "[variable-empty]")
{
  auto v = deferred::variable<int>();

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int&>);
  CHECK(v() == int{});

  SECTION("assignment")
  {
    v = 42;
    CHECK(v() == 42);
  }
}

TEST_CASE("initialized variable", "[variable-init]")
{
  constexpr auto v = deferred::variable(42);

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int const&>);
  CHECK(v() == 42);
}

TEST_CASE("constexpr variable", "[variable-constexpr]")
{
  constexpr auto v = deferred::variable(42);

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int const&>);
  static_assert(v() == 42);
}

TEST_CASE("variable from lambda", "[variable-from-lambda]")
{
  int i  = 0;
  auto v = deferred::variable([&] {
    ++i;
    return 10;
  });

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int&>);
  CHECK(i == 1);
  CHECK(v() == 10);
}

TEST_CASE("variable from mutable lambda", "[variable-from-mutable-lambda]")
{
  auto v = deferred::variable([i = 0]() mutable { return ++i; });

  static_assert(!deferred::is_constant_expression_v<decltype(v)>);
  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int&>);
  CHECK(v() == 1);
  CHECK(v() == 1);
}

namespace {

int function()
{
  return 10;
}

} // namespace

TEST_CASE("variable from function", "[variable-from-function]")
{
  auto v = deferred::variable(&function);

  static_assert(std::is_same_v<decltype(v)::value_type, int>);
  static_assert(std::is_same_v<decltype(v()), int&>);
  CHECK(v() == 10);
}

#if 0
TEST_CASE("unbound variable", "[variable-no-bind]")
{
  deferred::variable<int>();
  FAIL("Should not be able to create unbound variable");
}
#endif

#if 0
TEST_CASE("duplicate variable", "[variable-duplicate]")
{
  auto v1 = deferred::variable<int>();
  auto v2 = v1;
  v1 = 10;
  FAIL("Should not be able to duplicate variable");
}
#endif

TEST_CASE("variable named access", "[variable-get]")
{
  auto v = deferred::variable(42);

  static_assert(std::is_same_v<decltype(v.get()), int&>);
  static_assert(std::is_same_v<decltype(std::as_const(v).get()), int const&>);
  static_assert(std::is_same_v<decltype(std::move(v).get()), int>);
  static_assert(std::is_same_v<decltype(v.get()), decltype(v())>);
  static_assert(std::is_same_v<decltype(std::as_const(v).get()), decltype(std::as_const(v)())>);
  static_assert(std::is_same_v<decltype(std::move(v).get()), decltype(std::move(v)())>);
  static_assert(noexcept(v.get()));
  static_assert(noexcept(std::as_const(v).get()));
  static_assert(noexcept(std::move(v).get()));

  CHECK(v.get() == 42);
  CHECK(&v.get() == &v());
  CHECK(&std::as_const(v).get() == &v());

  SECTION("mutation")
  {
    v.get() = 7;
    CHECK(v() == 7);
    CHECK(std::as_const(v).get() == 7);
  }

  SECTION("move access")
  {
    auto s = deferred::variable(std::string("a string long enough to be allocated"));

    auto moved = std::move(s).get();
    CHECK(moved == "a string long enough to be allocated");
  }

  SECTION("constexpr")
  {
    constexpr auto c = deferred::variable(42);
    static_assert(c.get() == 42);
  }
}

TEST_CASE("variable deduces its type from a value", "[variable-ctad]")
{
  deferred::variable v{42};
  static_assert(std::is_same_v<decltype(v), deferred::variable<int>>);
  static_assert(std::is_same_v<decltype(deferred::variable(42)), deferred::variable<int>>);
  CHECK(v() == 42);

  // Neither a reference nor a cv-qualifier is retained: the variable owns its value.
  int i        = 1;
  int const ci = 2;
  int& r       = i;
  static_assert(std::is_same_v<decltype(deferred::variable{i}), deferred::variable<int>>);
  static_assert(std::is_same_v<decltype(deferred::variable{ci}), deferred::variable<int>>);
  static_assert(std::is_same_v<decltype(deferred::variable{r}), deferred::variable<int>>);
  static_assert(
    std::is_same_v<decltype(deferred::variable{"text"}), deferred::variable<char const*>>);

  deferred::variable owned{i};
  i = 100;
  CHECK(owned() == 1);
}

TEST_CASE("variable with a named type needs no initial value", "[variable-ctad]")
{
  deferred::variable<int> v;
  static_assert(std::is_same_v<decltype(deferred::variable<int>()), deferred::variable<int>>);
  CHECK(v() == 0);

  deferred::variable<long> converted{1};
  static_assert(std::is_same_v<decltype(converted()), long&>);
  CHECK(converted() == 1L);

  static_assert(!std::is_constructible_v<deferred::variable<int>, std::string>);
}

TEST_CASE("variable deduces the result type of a callable and stores the result", "[variable-ctad]")
{
  auto calls = 0;
  auto count = [&calls] {
    return ++calls;
  };

  deferred::variable v{count};
  static_assert(std::is_same_v<decltype(v), deferred::variable<int>>);
  CHECK(calls == 1);
  CHECK(v() == 1);

  // A const callable is evaluated too, not stored.
  auto const fixed = [] {
    return 7;
  };
  static_assert(std::is_same_v<decltype(deferred::variable{fixed}), deferred::variable<int>>);

  // A deferred expression is a callable like any other.
  deferred::constant c{5};
  deferred::variable from_constant{c};
  static_assert(std::is_same_v<decltype(from_constant), deferred::variable<int>>);
  CHECK(from_constant() == 5);
}

TEST_CASE("how a variable is copied", "[variable-ctad]")
{
  using V = deferred::variable<std::string>;

  V v{std::string("value")};
  V const cv{std::string("const value")};

  SECTION("from a non-const variable, by naming it")
  {
    deferred::variable w{v};
    auto x = deferred::variable(v);
    static_assert(std::is_same_v<decltype(w), V>);
    static_assert(std::is_same_v<decltype(x), V>);
    static_assert(std::is_constructible_v<V, V&>);

    // The copy is independent of its source.
    w = std::string("changed");
    CHECK(v() == "value");
    CHECK(w() == "changed");
    CHECK(x() == "value");

    // A variable of another type works the same way.
    deferred::variable<int> narrow{3};
    deferred::variable<long> wide{narrow};
    CHECK(wide() == 3L);
  }

  SECTION("from a const variable, through its value")
  {
    deferred::variable w{cv()};
    deferred::variable x{cv.get()};
    static_assert(std::is_same_v<decltype(w), V>);
    CHECK(w() == "const value");
    CHECK(x() == "const value");

    // Naming a const variable selects the deleted copy constructor.
    static_assert(!std::is_constructible_v<V, V const&>);
  }

  SECTION("from a temporary, through its moved value")
  {
    deferred::variable w{std::move(v)()};
    static_assert(std::is_same_v<decltype(w), V>);
    CHECK(w() == "value");
    CHECK(v().empty());

    // Naming a temporary selects the deleted move constructor.
    static_assert(!std::is_constructible_v<V, V&&>);
    static_assert(!std::is_constructible_v<V, V const&&>);
  }

  SECTION("into an existing variable, by assigning the value")
  {
    V w;
    w = v();
    CHECK(w() == "value");

    // Assigning the variable itself builds an expression that assigns when evaluated.
    auto later = (w = cv());
    static_assert(!deferred::deferred_expression<decltype(later)>);
    auto deferred_assignment = (w = v);
    v                        = std::string("newer");
    CHECK(w() == "const value");
    deferred_assignment();
    CHECK(w() == "newer");

    // Copy and move assignment are deleted.
    static_assert(!std::is_assignable_v<V&, V const&>);
    static_assert(!std::is_assignable_v<V&, V&&>);
  }

  SECTION("never implicitly")
  {
    static_assert(!std::is_copy_constructible_v<V>);
    static_assert(!std::is_move_constructible_v<V>);

    // `auto w = v;` and returning a named variable.
    static_assert(!std::is_convertible_v<V&, V>);
    static_assert(!std::is_convertible_v<V const&, V>);
    static_assert(!std::is_convertible_v<V&&, V>);

    // Passing a variable by value; by reference is the way.
    static_assert(!std::is_invocable_v<void (*)(V), V&>);
    static_assert(!std::is_invocable_v<void (*)(V), V const&>);
    static_assert(!std::is_invocable_v<void (*)(V), V>);
    static_assert(std::is_invocable_v<void (*)(V&), V&>);
    static_assert(std::is_invocable_v<void (*)(V const&), V const&>);
  }
}

TEST_CASE("a variable of a move-only type is not built from a variable", "[variable-ctad]")
{
  using V = deferred::variable<std::unique_ptr<int>>;

  // These must be false, not ill-formed.
  static_assert(!std::is_constructible_v<V, V&>);
  static_assert(!std::is_constructible_v<V, V const&>);
  static_assert(!std::is_constructible_v<V, V&&>);

  V v{std::make_unique<int>(5)};
  deferred::variable moved{std::move(v)()};
  REQUIRE(moved() != nullptr);
  CHECK(*moved() == 5);
  CHECK(v() == nullptr);
}

TEST_CASE("a variable with a named type is built from a braced list", "[variable-ctad]")
{
  deferred::variable<std::vector<int>> v({1, 2, 3});
  CHECK(v().size() == 3);

  deferred::variable<std::pair<int, int>> p({1, 2});
  CHECK(p().second == 2);
}

TEST_CASE("a variable stores a value that is not callable without evaluating it", "[variable-ctad]")
{
  // A view built from an lvalue owner refers to the owner, not to a temporary copy.
  std::vector<int> numbers{1, 2, 3};
  deferred::variable<std::span<int const>> view{numbers};
  CHECK(view().data() == numbers.data());

  // A variable named as holding a callable stores it.
  deferred::variable<std::function<int()>> stored{[] { return 5; }};
  CHECK(stored()() == 5);

  // An lvalue that cannot be copied is a failed requirement, not an error.
  static_assert(!std::is_constructible_v<deferred::variable<int>, std::mutex&>);
}
