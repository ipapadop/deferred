// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <type_traits>

#include "deferred/detail/expression_reference.hpp"

namespace {

struct self
{ };

struct child
{ };

template<typename Self, typename Expression>
using ref_t = deferred::detail::expression_reference_t<Self, Expression>;

} // namespace

TEST_CASE("expression_reference_t yields a mutable reference for a mutable self", "[expr-ref]")
{
  STATIC_CHECK(std::is_same_v<ref_t<self&, child>, child&>);
  STATIC_CHECK(std::is_same_v<ref_t<self&&, child>, child&>);
}

TEST_CASE("expression_reference_t yields a const reference for a const self", "[expr-ref]")
{
  STATIC_CHECK(std::is_same_v<ref_t<self const&, child>, child const&>);
  STATIC_CHECK(std::is_same_v<ref_t<self const&&, child>, child const&>);
}

TEST_CASE("expression_reference_t handles a non-reference self", "[expr-ref]")
{
  STATIC_CHECK(std::is_same_v<ref_t<self, child>, child&>);
  STATIC_CHECK(std::is_same_v<ref_t<self const, child>, child const&>);
}

TEST_CASE("expression_reference_t collapses an already-reference expression", "[expr-ref]")
{
  STATIC_CHECK(std::is_same_v<ref_t<self&, child&>, child&>);
  STATIC_CHECK(std::is_same_v<ref_t<self const&, child&>, child&>);
  STATIC_CHECK(std::is_same_v<ref_t<self&, child const&>, child const&>);
}

TEST_CASE("expression_reference_t preserves a const expression type", "[expr-ref]")
{
  STATIC_CHECK(std::is_same_v<ref_t<self&, child const>, child const&>);
  STATIC_CHECK(std::is_same_v<ref_t<self const&, child const>, child const&>);
}
