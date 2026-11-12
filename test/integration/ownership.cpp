// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "deferred/deferred.hpp"

namespace {

/// @brief Counts how often it is copied and moved.
struct counted
{
  int* copies;
  int* moves;
  int value;

  counted(int& c, int& m, int v) : copies(&c), moves(&m), value(v)
  { }

  counted(counted const& other) : copies(other.copies), moves(other.moves), value(other.value)
  {
    ++*copies;
  }

  counted(counted&& other) noexcept : copies(other.copies), moves(other.moves), value(other.value)
  {
    ++*moves;
  }

  friend int operator+(counted const& left, counted const& right)
  {
    return left.value + right.value;
  }
};

// The strings are long enough to be allocated, so ASan sees a use after the function
// returns.
auto concatenation_of_locals()
{
  auto c      = deferred::constant(std::string(40, 'a'));
  auto suffix = deferred::constant(std::string(40, 'b'));
  auto c2     = c + suffix;
  auto c3     = c2 + c;
  return c3;
}

auto scaled(deferred::variable<int>& v)
{
  auto factor = deferred::constant(3);
  auto ex     = v * factor;
  return ex;
}

auto sum_below(deferred::variable<int>& i, deferred::variable<int>& total, int limit)
{
  auto bound = deferred::constant(limit);
  auto body  = total += i;
  auto loop  = deferred::for_(i = deferred::constant(0), i < bound, ++i).do_(body);
  return loop;
}

auto choice(deferred::variable<int>& v)
{
  auto small = deferred::constant(std::string(40, 's'));
  auto large = deferred::constant(std::string(40, 'l'));
  auto limit = deferred::constant(10);
  auto ex    = deferred::if_(v < limit).then_(small).else_(large);
  return ex;
}

} // namespace

TEST_CASE("an expression built from local constants can be returned", "[ownership]")
{
  auto ex = concatenation_of_locals();

  CHECK(ex() == std::string(40, 'a') + std::string(40, 'b') + std::string(40, 'a'));
}

TEST_CASE("a returned expression still refers to the caller's variable", "[ownership]")
{
  deferred::variable v{2};
  auto ex = scaled(v);

  CHECK(ex() == 6);
  v = 5;
  CHECK(ex() == 15);
}

TEST_CASE("a loop built in a function can be returned", "[ownership]")
{
  deferred::variable<int> i;
  deferred::variable<int> total;
  auto loop = sum_below(i, total, 5);

  loop();
  CHECK(total() == 10);
  loop();
  CHECK(total() == 20);
}

TEST_CASE("a conditional built from local branches can be returned", "[ownership]")
{
  deferred::variable v{1};
  auto ex = choice(v);

  CHECK(ex() == std::string(40, 's'));
  v = 20;
  CHECK(ex() == std::string(40, 'l'));
}

TEST_CASE("a named operand is copied once for each use", "[ownership]")
{
  int copies = 0;
  int moves  = 0;
  auto c     = deferred::constant(counted(copies, moves, 1));
  copies     = 0;
  moves      = 0;

  auto twice = c + c;
  CHECK(copies == 2);
  CHECK(twice() == 2);

  // An operand that is no longer needed is moved in.
  copies     = 0;
  moves      = 0;
  auto moved = std::move(c) + deferred::constant(counted(copies, moves, 2));
  CHECK(copies == 0);
  CHECK(moved() == 3);
}

TEST_CASE("a builder constructs the operand it is given in place", "[ownership]")
{
  int copies = 0;
  int moves  = 0;
  auto c     = deferred::constant(counted(copies, moves, 1));
  auto flag  = deferred::constant(true);
  auto i     = deferred::variable(0);

  auto reset = [&] {
    copies = 0;
    moves  = 0;
  };

  // The operand handed to the last call of a chain is copied once and never moved.
  reset();
  auto with_then = deferred::if_(flag).then_(c);
  CHECK(copies == 1);
  CHECK(moves == 0);

  reset();
  [[maybe_unused]] auto with_else = deferred::if_(flag).then_(1).else_(c);
  CHECK(copies == 1);
  CHECK(moves == 0);

  reset();
  [[maybe_unused]] auto extended = with_then.else_(c);
  CHECK(copies == 2);
  CHECK(moves == 0);

  reset();
  [[maybe_unused]] auto with_default = deferred::switch_(i).default_(c);
  CHECK(copies == 1);
  CHECK(moves == 0);

  reset();
  [[maybe_unused]] auto with_case = deferred::switch_(i).case_(1).then_(c);
  CHECK(copies == 1);
  CHECK(moves == 0);

  reset();
  [[maybe_unused]] auto case_and_default = deferred::switch_(i).case_(1).then_(2).default_(c);
  CHECK(copies == 1);
  CHECK(moves == 0);
}

TEST_CASE("a reference from a temporary expression over a named constant is safe", "[ownership]")
{
  auto table =
    deferred::constant(std::vector<std::string>{std::string(40, 'a'), std::string(40, 'b')});
  auto index = deferred::variable(std::size_t{1});

  // The temporary expression owns a copy of the table and dies with the statement, so
  // it yields the element by value.
  auto const& element = deferred::subscript(table, index)();
  CHECK(element == std::string(40, 'b'));

  auto text                = deferred::constant(std::optional<std::string>(std::string(40, 'x')));
  std::string const& value = (*text)();
  CHECK(value == std::string(40, 'x'));
}

TEST_CASE("evaluating a temporary expression does not move its operands out", "[ownership]")
{
  int copies = 0;
  int moves  = 0;
  auto c     = deferred::constant(counted(copies, moves, 1));
  auto sum   = c + c;
  copies     = 0;
  moves      = 0;

  // The operands are read where they are stored, as for a stored expression.
  CHECK(std::move(sum)() == 2);
  CHECK(copies == 0);
  CHECK(moves == 0);
  CHECK((c + c)() == 2);
  CHECK(moves == 0);
}

TEST_CASE("a callable that returns a reference to its argument works in a temporary expression",
          "[ownership]")
{
  auto same = [](std::string const& text) -> std::string const& {
    return text;
  };

  // The argument lives in the temporary expression until the end of the statement, so
  // the reference the callable returns can still be read; ASan checks that.
  std::string const copy = deferred::invoke(same, deferred::constant(std::string(40, 'r')))();
  CHECK(copy == std::string(40, 'r'));

  auto named                = deferred::constant(std::string(40, 'n'));
  std::string const through = deferred::invoke(same, named)();
  CHECK(through == std::string(40, 'n'));
}

TEST_CASE("a variable of a const type and a const variable are both read through", "[ownership]")
{
  deferred::variable<int const> fixed{5};
  deferred::variable<int> const frozen{7};
  deferred::variable<int> v{1};

  auto sum = fixed + frozen + v;
  CHECK(sum() == 13);
  v = 2;
  CHECK(sum() == 14);
}

namespace {

/// @brief A user-written node that holds state of its own.
struct cell
{
  using subexpression_types = std::tuple<>;

  static constexpr bool mutable_state = true;

  int value = 0;

  int& operator()() noexcept
  {
    return value;
  }

  int const& operator()() const noexcept
  {
    return value;
  }

  auto subexpressions() const noexcept
  {
    return std::tuple<>{};
  }
};

} // namespace

TEST_CASE("an explicit copy of an expression copies the state it owns", "[ownership]")
{
  deferred::variable v{1};
  auto calls = 0;

  // The callable is given to the expression, which then owns its counter.
  auto ex   = deferred::invoke([count = 0, &calls]() mutable {
              ++calls;
              return ++count;
              })
              + v;
  auto copy = ex;

  // State a callable holds by value belongs to each copy; the variable is shared.
  CHECK(ex() == 2);
  CHECK(ex() == 3);
  CHECK(copy() == 2);
  v = 10;
  CHECK(ex() == 13);
  CHECK(copy() == 12);

  // State captured by reference is shared too.
  CHECK(calls == 5);
}

TEST_CASE("std::ref shares a callable that has state between expressions", "[ownership]")
{
  auto next = deferred::invoke([count = 0]() mutable { return ++count; });

  auto first  = std::ref(next) + deferred::constant(0);
  auto second = std::ref(next) + deferred::constant(0);

  CHECK(first() == 1);
  CHECK(second() == 2);
  CHECK(next() == 3);

  // A const expression can still call it, because it refers to the callable.
  auto const fixed = std::ref(next) + deferred::constant(10);
  CHECK(fixed() == 14);
}

TEST_CASE("std::move gives a callable that has state to one expression", "[ownership]")
{
  auto next = deferred::invoke([count = 0]() mutable { return ++count; });
  auto ex   = std::move(next) + deferred::constant(0);

  CHECK(ex() == 1);
  CHECK(ex() == 2);
}

TEST_CASE("std::ref lets an expression read and write a user node that has state", "[ownership]")
{
  // The node is not in namespace deferred, so the operators are not found through it.
  using namespace deferred;

  cell c;

  auto read = std::ref(c) + deferred::constant(1);
  c.value   = 5;
  CHECK(read() == 6);

  auto increment = ++std::ref(c);
  increment();
  increment();
  CHECK(c.value == 7);

  auto add = (std::ref(c) += deferred::constant(3));
  add();
  CHECK(c.value == 10);

  deferred::assign(std::ref(c), deferred::constant(1))();
  CHECK(c.value == 1);

  // std::cref gives read-only access.
  auto view = std::cref(c) * deferred::constant(2);
  c.value   = 4;
  CHECK(view() == 8);
}

TEST_CASE("std::ref works for a variable too", "[ownership]")
{
  deferred::variable v{1};

  auto doubled = std::ref(v) * deferred::constant(2);
  v            = 5;
  CHECK(doubled() == 10);

  (++std::ref(v))();
  CHECK(v() == 6);
}

TEST_CASE("expressions can be collected and outlive what built them", "[ownership]")
{
  deferred::variable x{2};
  std::vector<decltype(scaled(x))> expressions;

  for (auto k = 0; k < 4; ++k)
  {
    expressions.push_back(scaled(x));
  }

  x = 3;
  for (auto const& ex : expressions)
  {
    CHECK(ex() == 9);
  }
}
