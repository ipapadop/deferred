// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

TEST_CASE("for built entirely from deferred clauses", "[for-body]")
{
  auto i   = deferred::variable<int>();
  auto sum = 0;

  // Assigning a deferred expression to a variable builds the init clause, so no
  // clause needs a lambda wrapper.
  auto ex = deferred::for_(i = deferred::constant(0), i < 5, ++i).do_([&] { sum += i(); });
  ex();
  CHECK(i() == 5);
  CHECK(sum == 10);
}

TEST_CASE("for re-runs its initialization on every evaluation", "[for-init]")
{
  auto i     = deferred::variable(100);
  auto count = 0;

  auto ex = deferred::for_(i = deferred::constant(0), i != 3, ++i).do_([&count] { ++count; });
  ex();
  ex();
  CHECK(i() == 3);
  CHECK(count == 6);
}

TEST_CASE("for with a nested conditional body", "[for-nested-conditional]")
{
  auto i       = deferred::variable<int>();
  auto evens   = 0;
  auto counter = deferred::if_([&i] { return i() % 2 == 0; }).then_([&evens] { ++evens; });

  auto ex = deferred::for_([&i] { i = 0; }, i < 5, ++i).do_([&] { counter(); });
  ex();
  CHECK(i() == 5);
  CHECK(evens == 3);
}

TEST_CASE("for with a nested do-while body", "[for-nested-do-while]")
{
  auto i     = deferred::variable<int>();
  auto j     = deferred::variable<int>();
  auto total = 0;

  auto inner = deferred::do_([&] {
                 ++total;
                 j = j() - 1;
               }).while_(j != 0);
  auto ex    = deferred::for_([&] { i = 0; }, i < 3, ++i).do_([&] {
    j = 2;
    inner();
  });

  ex();
  CHECK(i() == 3);
  CHECK(total == 6);
}


TEST_CASE("for init assigns one variable to another by value", "[for-init]")
{
  auto i     = deferred::variable<int>();
  auto start = deferred::variable(2);
  auto seen  = 0;

  auto ex = deferred::for_(i = start, i < 5, ++i).do_([&] { seen += i(); });
  ex();
  CHECK(seen == 2 + 3 + 4);

  // The loop counter took a copy: running it to completion left `start` alone.
  CHECK(i() == 5);
  CHECK(start() == 2);
}
