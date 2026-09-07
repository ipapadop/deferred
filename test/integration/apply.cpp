// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <type_traits>

#include "deferred/deferred.hpp"

namespace {

struct widget
{
  int value;

  int get() const noexcept
  {
    return value;
  }

  int add(int increment) noexcept
  {
    value += increment;
    return value;
  }
};

} // namespace

TEST_CASE("apply with constants", "[apply-constants]")
{
  auto c1 = deferred::constant(10);
  auto c2 = deferred::constant(10);
  auto ex = c1 + c2;

  CHECK(deferred::apply(std::plus<int>{}, std::make_tuple(c1, c2))() == ex());
}

TEST_CASE("invoke member pointers", "[invoke-member-pointer]")
{
  widget object{40};

  auto owned   = deferred::invoke(&widget::get, object);
  object.value = 41;
  CHECK(owned() == 40);

  auto referenced = deferred::invoke(&widget::add, std::ref(object), 1);
  CHECK(referenced() == 42);
  CHECK(object.value == 42);

  auto data = deferred::invoke(&widget::value, &object);
  static_assert(std::is_same_v<decltype(data()), int&>);
  data() = 43;
  CHECK(object.value == 43);
}
