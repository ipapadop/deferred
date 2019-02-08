/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <catch2/catch.hpp>

#include <type_traits>

#include "deferred/make_deferred.hpp"
#include "deferred/constant.hpp"
#include "deferred/expression.hpp"
#include "deferred/variable.hpp"

#include "deferred/type_name.hpp"

namespace
{

int foo(int, bool)
{
  return 0;
}

} // namespace

TEST_CASE("make_deferred constants", "[make-deferred-constants]")
{
  SECTION("int")
  {
    using type = deferred::make_deferred_t<int>;
    CHECK(std::is_same<type, deferred::constant_<int>>::value);
  }

  SECTION("int&")
  {
    using type = deferred::make_deferred_t<int&>;
    CHECK(std::is_same<type, deferred::constant_<int&>>::value);
  }

  SECTION("constant with literal")
  {
    auto c = deferred::constant(10);
    using type = deferred::make_deferred_t<decltype(c)>;
    CHECK(std::is_same<type, deferred::constant_<int>>::value);
  }

  SECTION("constant with reference")
  {
    auto i = 10;
    auto c = deferred::constant(i);
    using type = deferred::make_deferred_t<decltype(c)>;
    CHECK(std::is_same<type, deferred::constant_<int&>>::value);
  }
}

TEST_CASE("make_deferred expresssions", "[make-deferred-expressions]")
{
  SECTION("lambda")
  {
    auto f = [] { return 4; };
    using type = deferred::make_deferred_t<decltype(f)>;
    CHECK(std::is_same<type, deferred::expression_<decltype(f)>>::value);
  }
  foo(3, false);
#if 0
  // FAILS
  SECTION("function")
  {
    using type = deferred::make_deferred_t<decltype(foo)>;
    printf("%s\n", deferred::type_name<type>().c_str());
    CHECK(std::is_same<type, deferred::expression_<decltype(foo)>>::value);
  }
#endif
}

TEST_CASE("make_deferred variables", "[make-deferred-variables]")
{
  SECTION("uninitialized variable")
  {
    auto v = deferred::variable<int>();
    using type = deferred::make_deferred_t<decltype(v)>;
    CHECK(std::is_same<type, deferred::variable_<int>>::value);
  }

  SECTION("initialized variable")
  {
    auto v = deferred::variable(true);
    using type = deferred::make_deferred_t<decltype(v)>;
    CHECK(std::is_same<type, deferred::variable_<bool>>::value);
  }
}
