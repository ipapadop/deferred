// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "deferred/type_name.hpp"

namespace {

struct plain_type
{ };

} // namespace

TEST_CASE("type_name names a fundamental type", "[type-name]")
{
  CHECK(deferred::type_name<int>() == "int");
  CHECK(deferred::type_name<double>() == "double");
}

TEST_CASE("type_name keeps cv-qualifiers", "[type-name]")
{
  CHECK(deferred::type_name<int const>() == "int const");
  CHECK(deferred::type_name<int volatile>() == "int volatile");
  CHECK(deferred::type_name<int const volatile>() == "int const volatile");
}

TEST_CASE("type_name keeps reference qualifiers", "[type-name]")
{
  CHECK(deferred::type_name<int&>() == "int&");
  CHECK(deferred::type_name<int&&>() == "int&&");
  CHECK(deferred::type_name<int const&>() == "int const&");
}

TEST_CASE("type_name unmangles a user-defined type", "[type-name]")
{
  auto const name = deferred::type_name<plain_type>();

  CHECK(name.find("plain_type") != std::string::npos);
}
