// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT


#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "deferred/assign.hpp"
#include "deferred/constant.hpp"
#include "deferred/invoke.hpp"
#include "deferred/operators.hpp"
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

template<typename T, typename U>
concept compound_addable = requires(T t, U u) { t += u; };

/// @brief Counts how many times it is copied.
struct counted
{
  static inline int copies = 0;
  int value{};

  counted() = default;
  counted(counted const& other) : value(other.value)
  {
    ++copies;
  }
  counted& operator=(counted const&) = default;
};

template<typename Variable, typename Value>
concept chainable = requires(Variable variable, Value value) { variable = value; };

} // namespace

TEST_CASE("assigning a value stores it immediately", "[assign]")
{
  auto v = deferred::variable<int>();

  // The deferred overload must not capture a plain value assignment.
  static_assert(std::is_same_v<decltype(v = 1), deferred::detail::immediate_assignment>);
  v = 1;
  CHECK(v() == 1);
}

TEST_CASE("the result of assigning a value is not an expression", "[assign]")
{
  auto v = deferred::variable<int>();

  // Returning the variable would let `v = 1` pass as an expression that reads `v`, in
  // a branch, a body or a loop clause, after the assignment has already happened.
  static_assert(!deferred::deferred_expression<decltype(v = 1)>);
  static_assert(!std::is_invocable_v<decltype(v = 1)>);
  static_assert(deferred::deferred_expression<decltype(v = deferred::constant(1))>);
}

TEST_CASE("assigning a deferred expression builds an expression", "[assign]")
{
  auto v = deferred::variable<int>();
  auto e = (v = deferred::constant(7));

  static_assert(deferred::deferred_expression<decltype(e)>);
  static_assert(!deferred::is_constant_expression_v<decltype(e)>);
  CHECK(v() == 0);

  CHECK(e() == 7);
  CHECK(v() == 7);

  // The expression assigns again on every evaluation.
  v = 0;
  CHECK(e() == 7);
  CHECK(v() == 7);
}

TEST_CASE("a deferred assignment re-evaluates its right-hand side", "[assign]")
{
  auto v = deferred::variable<int>();
  auto j = deferred::variable(1);
  auto e = (v = j + 1);

  CHECK(e() == 2);
  j = 10;
  CHECK(e() == 11);
}

TEST_CASE("assigning a variable to a variable copies the value", "[assign]")
{
  auto i = deferred::variable<int>();
  auto j = deferred::variable(3);
  auto e = (i = j);

  e();
  CHECK(i() == 3);
  CHECK(j() == 3);

  // The two variables are independent afterwards, as with a built-in assignment.
  i = 99;
  CHECK(j() == 3);
  j = 7;
  CHECK(i() == 99);

  // Re-evaluating re-reads the right-hand side.
  e();
  CHECK(i() == 7);
}

TEST_CASE("assigning a variable holding a reference follows that type's assignment", "[assign]")
{
  auto a = 1;
  auto b = 2;
  auto i = deferred::variable(std::ref(a));
  auto j = deferred::variable(std::ref(b));

  (i = j)();

  // std::reference_wrapper rebinds rather than copying through.
  CHECK(&i().get() == &b);
  CHECK(a == 1);
  CHECK(b == 2);

  i().get() = 42;
  CHECK(b == 42);
  CHECK(a == 1);
}


TEST_CASE("deferred assignment carries its exception guarantee", "[assign]")
{
  STATIC_CHECK(noexcept(std::declval<deferred::variable_<int>&>() = deferred::constant_<int>(0)));
  STATIC_CHECK(!noexcept(std::declval<deferred::variable_<throwing_value>&>() =
                           std::declval<deferred::constant_<throwing_value>>()));
}

TEST_CASE("a deferred assignment cannot bind to a temporary variable", "[assign]")
{
  // The operator is &-qualified, so it cannot capture a reference to a temporary.
  static_assert(std::is_assignable_v<deferred::variable_<int>&, deferred::constant_<int>>);
  static_assert(!std::is_assignable_v<deferred::variable_<int>, deferred::constant_<int>>);
}

TEST_CASE("compound assignment builds an expression", "[assign-compound]")
{
  auto v = deferred::variable(10);
  auto e = (v += 5);

  static_assert(deferred::deferred_expression<decltype(e)>);
  static_assert(!deferred::is_constant_expression_v<decltype(e)>);
  CHECK(v() == 10);

  CHECK(e() == 15);
  CHECK(v() == 15);

  // It assigns again on every evaluation.
  CHECK(e() == 20);
}

TEST_CASE("every compound assignment operator is deferred", "[assign-compound]")
{
  auto v = deferred::variable(10);

  (v += 5)();
  CHECK(v() == 15);
  (v -= 3)();
  CHECK(v() == 12);
  (v *= 2)();
  CHECK(v() == 24);
  (v /= 4)();
  CHECK(v() == 6);
  (v %= 4)();
  CHECK(v() == 2);
  (v <<= 3)();
  CHECK(v() == 16);
  (v >>= 2)();
  CHECK(v() == 4);
  (v &= 6)();
  CHECK(v() == 4);
  (v |= 3)();
  CHECK(v() == 7);
  (v ^= 5)();
  CHECK(v() == 2);
}

TEST_CASE("compound assignment uses the type's own operator", "[assign-compound]")
{
  auto s = deferred::variable<std::string>(std::string("ab"));

  // std::string has no operator+ for this, but it does have operator+=.
  (s += std::string("cd"))();
  CHECK(s() == "abcd");
}

TEST_CASE("compound assignment re-reads its right-hand side", "[assign-compound]")
{
  auto acc  = deferred::variable(0);
  auto step = deferred::variable(2);
  auto e    = (acc += step);

  e();
  e();
  CHECK(acc() == 4);

  step = 10;
  e();
  CHECK(acc() == 14);
}

TEST_CASE("compound assignment requires a deferred target", "[assign-compound]")
{
  static_assert(compound_addable<deferred::variable_<int>&, int>);
  static_assert(!compound_addable<int, deferred::variable_<int>&>);
}

TEST_CASE("only a plain lvalue variable is a deferred assignment source", "[assign]")
{
  using target = deferred::variable_<int>;

  // `i = j` builds a deferred assignment.
  static_assert(std::is_assignable_v<target&, target&>);

  // The other value categories are rejected. variable_ is deliberately non-copyable,
  // and its user-declared move constructor makes the implicit copy assignment deleted;
  // being a non-template, that wins overload resolution against this operator. The
  // rejection is the right outcome anyway: a deferred assignment reads its right-hand
  // side when it is evaluated, so the value category at build time carries no meaning.
  static_assert(!std::is_assignable_v<target&, target const&>);
  static_assert(!std::is_assignable_v<target&, target&&>);

  // Every other deferred right-hand side is unaffected by this.
  static_assert(std::is_assignable_v<target&, deferred::constant_<int>>);
  static_assert(std::is_assignable_v<target&, deferred::constant_<int>&>);
}

TEST_CASE("assign reaches a target that is not a variable", "[assign]")
{
  struct counter
  {
    int value{};
  };

  counter c;
  auto source = deferred::variable(1);
  auto ex     = deferred::assign(deferred::invoke(&counter::value, &c), source);

  ex();
  CHECK(c.value == 1);

  // The value expression is read on every evaluation, as for a variable target.
  source = 9;
  ex();
  CHECK(c.value == 9);
}

namespace {

template<typename Target, typename Value>
concept assignable_target = requires(Target t, Value v) { deferred::assign(t, v); };

} // namespace

TEST_CASE("an assignment target must yield an lvalue", "[assign]")
{
  using lvalue_target = deferred::variable_<int>&;
  // Evaluating this yields a prvalue: assigning would write into a temporary and hand
  // back a reference to it.
  using prvalue_target = decltype(deferred::invoke([] { return 0; }))&;

  static_assert(assignable_target<lvalue_target, deferred::constant_<int>>);
  static_assert(!assignable_target<prvalue_target, deferred::constant_<int>>);

  static_assert(compound_addable<lvalue_target, int>);
  static_assert(!compound_addable<prvalue_target, int>);
}



TEST_CASE("an immediate assignment cannot be chained", "[assign]")
{
  using V = deferred::variable_<int>;

  // `b = 7` has already assigned and yields no value, so there is nothing for the outer
  // `=` to assign; the deferred spelling chains.
  static_assert(!chainable<V&, decltype(std::declval<V&>() = 7)>);
  static_assert(chainable<V&, decltype(std::declval<V&>() = deferred::constant(7))>);

  auto a = deferred::variable<int>();
  auto b = deferred::variable<int>();
  a      = 1;

  auto chained = (a = (b = deferred::constant(7)));

  CHECK(b() == 0);
  CHECK(a() == 1);
  chained();
  CHECK(b() == 7);
  CHECK(a() == 7);
}

TEST_CASE("a deferred assignment does not copy a referenced right-hand side", "[assign]")
{
  auto target = deferred::variable<counted>();
  auto source = deferred::variable(counted{});
  auto ex     = (target = source);

  counted::copies = 0;
  ex();
  ex();

  // The source already yields a reference, so evaluate_ref hands it straight to the
  // assignment instead of decaying it into a temporary first.
  CHECK(counted::copies == 0);
}

TEST_CASE("a prvalue right-hand side is moved into the target", "[assign]")
{
  auto p  = deferred::variable<std::unique_ptr<int>>();
  auto ex = deferred::assign(p, deferred::invoke([] { return std::make_unique<int>(3); }));

  ex();
  REQUIRE(p() != nullptr);
  CHECK(*p() == 3);
}

TEST_CASE("a scalar right-hand side is read before the target is evaluated", "[assign]")
{
  auto i = deferred::variable(1);

  // As the built-in (i = 5) += i does: the right operand is the i from before the
  // inner assignment.
  auto inner = (i = deferred::constant(5));
  auto ex    = (inner += i);
  ex();
  CHECK(i() == 6);
}

TEST_CASE("a const target is rejected by the constraint", "[assign]")
{
  using const_target = deferred::constant_<int>&;

  // These must be answerable as false rather than fail inside the operation.
  static_assert(!assignable_target<const_target, deferred::constant_<int>>);
  static_assert(!compound_addable<const_target, int>);
}

TEST_CASE("assigning from a variable copies when the type cannot be moved", "[assign]")
{
  struct copy_only
  {
    int value = 0;

    copy_only()                            = default;
    copy_only(copy_only const&)            = default;
    copy_only& operator=(copy_only const&) = default;
    copy_only& operator=(copy_only&&)      = delete;
  };

  auto i    = deferred::variable<copy_only>();
  auto j    = deferred::variable<copy_only>();
  j().value = 4;

  auto ex = (i = j);
  ex();
  CHECK(i().value == 4);
}
