// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <vector>

#include "deferred/deferred.hpp"

TEST_CASE("deferred assignment composes with operators", "[assign-operators]")
{
  auto v = deferred::variable<int>();
  auto n = deferred::variable(4);

  auto ex = (v = n * 2 + 1);
  ex();
  CHECK(v() == 9);

  n = 10;
  ex();
  CHECK(v() == 21);
}

TEST_CASE("deferred assignment as a loop body", "[assign-loop]")
{
  auto n  = deferred::variable(5);
  auto ex = deferred::while_(n != 0).do_(n = n - 1);

  ex();
  CHECK(n() == 0);
}

TEST_CASE("deferred assignment inside a conditional branch", "[assign-conditional]")
{
  auto n    = deferred::variable(0);
  auto flag = deferred::variable(true);
  auto ex   = deferred::if_(flag).then_(n = deferred::constant(1)).else_(n = deferred::constant(2));

  // The branch yields the assigned value, as a built-in assignment would.
  CHECK(ex() == 1);
  CHECK(n() == 1);

  flag = false;
  CHECK(ex() == 2);
  CHECK(n() == 2);
}

TEST_CASE("compound assignment as a loop step and body", "[assign-compound]")
{
  auto i   = deferred::variable<int>();
  auto sum = deferred::variable(0);

  // No lambda in any clause.
  auto ex = deferred::for_(i = deferred::constant(0), i < 5, i += 1).do_(sum += i);
  ex();
  CHECK(i() == 5);
  CHECK(sum() == 0 + 1 + 2 + 3 + 4);
}

TEST_CASE("compound assignment composes with operators", "[assign-compound]")
{
  auto v = deferred::variable(1);
  auto n = deferred::variable(3);

  auto ex = (v *= n + 1);
  ex();
  CHECK(v() == 4);

  n = 2;
  ex();
  CHECK(v() == 12);
}

TEST_CASE("assign() targets an lvalue that is not a variable", "[assign-function]")
{
  struct counter
  {
    int value = 0;
  };

  auto c = counter{};
  auto n = deferred::variable(3);

  auto ex = deferred::for_(n = deferred::constant(3), n != 0, --n)
              .do_(deferred::assign(deferred::invoke(&counter::value, &c),
                                    deferred::invoke(&counter::value, &c) + n));
  ex();
  CHECK(c.value == 6);
  ex();
  CHECK(c.value == 12);
}

TEST_CASE("assign() into a subscripted element", "[assign-function]")
{
  auto values = deferred::variable(std::vector<int>(3));
  auto i      = deferred::variable<std::size_t>();

  auto ex = deferred::for_(i = deferred::constant(std::size_t{0}), i < 3, ++i)
              .do_(deferred::assign(deferred::subscript(values, i), i * 10));
  ex();
  CHECK(values() == std::vector<int>{0, 10, 20});
}

TEST_CASE("a constant assigned in a loop body is assigned by the loop", "[assign-deferred-value]")
{
  auto n    = deferred::variable(3);
  auto seen = deferred::variable(false);
  auto loop = deferred::while_(n != 0).do_(deferred::seq(seen = deferred::constant(true), n -= 1));

  // Building the loop does not set the flag.
  CHECK(seen() == false);

  loop();
  CHECK(seen() == true);
  CHECK(n() == 0);

  // With no iterations, the flag is left alone.
  seen = false;
  loop();
  CHECK(seen() == false);
}

TEST_CASE("a constant assigned in a branch is assigned when the branch is taken",
          "[assign-deferred-value]")
{
  auto i     = deferred::variable(12);
  auto reset = deferred::if_(i > 10).then_(i = deferred::constant(0)).else_(i = i + 1);

  CHECK(i() == 12);

  reset();
  CHECK(i() == 0);

  reset();
  CHECK(i() == 1);
}

TEST_CASE("immediate and deferred assignment combine", "[assign-deferred-value]")
{
  auto key  = deferred::variable(2);
  auto name = deferred::variable(std::string{});
  auto pick = deferred::switch_(key)
                .case_(1)
                .then_(name = deferred::constant(std::string("one")))
                .case_(2)
                .then_(name = deferred::constant(std::string("two")))
                .default_(name = deferred::constant(std::string("many")));

  CHECK(name().empty());

  pick();
  CHECK(name() == "two");

  // The inputs of an expression are set with an immediate assignment.
  key = 9;
  pick();
  CHECK(name() == "many");
}
