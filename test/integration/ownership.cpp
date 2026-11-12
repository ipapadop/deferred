// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <string>
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

TEST_CASE("a copy of an expression is independent except for its variables", "[ownership]")
{
  deferred::variable v{1};
  auto calls   = 0;
  auto counter = deferred::invoke([count = 0, &calls]() mutable {
    ++calls;
    return ++count;
  });

  auto ex   = counter + v;
  auto copy = ex;

  // State a callable holds by value belongs to each copy; the variable is shared.
  CHECK(ex() == 2);
  CHECK(ex() == 3);
  CHECK(copy() == 2);
  v = 10;
  CHECK(ex() == 13);
  CHECK(copy() == 12);

  // State captured by reference is shared too, and the named node was not touched.
  CHECK(calls == 5);
  CHECK(counter() == 1);
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
