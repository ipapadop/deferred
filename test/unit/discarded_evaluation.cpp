// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/deferred.hpp"

// Evaluating an expression for its effect must not warn, so a discarded result is an
// error in this file: a [[nodiscard]] on the wrong operator() stops it compiling.
#if defined(__clang__) || defined(__GNUC__)
#  pragma GCC diagnostic error "-Wunused-result"
#elif defined(_MSC_VER)
#  pragma warning(error : 4834)
#endif

TEST_CASE("an expression can be evaluated for its effect", "[discarded-evaluation]")
{
  auto i = deferred::variable(0);
  auto j = deferred::variable(0);

  // Increment, compound assignment and assignment are interchangeable as statements.
  (++i)();
  (i += 1)();
  (i = i + 1)();
  CHECK(i() == 3);

  // So is every node that may contain one of them.
  deferred::seq(++i, ++j)();
  (i > 0 && ++j > 0)();
  deferred::if_(i > 0).then_(++j).else_(--j)();
  deferred::switch_(i).case_(4).then_(++j).default_(--j)();
  deferred::invoke([](int& value) { return ++value; }, j)();
  CHECK(i() == 4);
  CHECK(j() == 5);
}

TEST_CASE("eval can be called for its effect", "[discarded-evaluation]")
{
  auto i = deferred::variable(0);
  auto j = deferred::variable(0);

  (++i).eval();
  (i += 1).eval();
  (i = i + 1).eval();
  deferred::seq(++i, ++j).eval();
  (i > 0 && ++j > 0).eval();
  deferred::if_(i > 0).then_(++j).else_(--j).eval();
  deferred::switch_(i).case_(4).then_(++j).default_(--j).eval();
  deferred::invoke([](int& value) { return ++value; }, j).eval();
  CHECK(i() == 4);
  CHECK(j() == 5);
}

TEST_CASE("the evaluation functions can be called for their effect", "[discarded-evaluation]")
{
  auto i = deferred::variable(0);

  deferred::evaluate(++i);
  deferred::evaluate_ref(++i);
  deferred::recursive_evaluate(++i);
  CHECK(i() == 3);
}
