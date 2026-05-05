// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <type_traits>
#include <variant>

#include "deferred/type_traits/homogenized_type.hpp"

TEST_CASE("homogenized_type with same types", "[homogenized_type]")
{
  using type = deferred::homogenized_type_t<int, int, int>;
  STATIC_CHECK(std::is_same_v<type, int>);
}

TEST_CASE("homogenized_type with empty list", "[homogenized_type]")
{
  using type = deferred::homogenized_type_t<>;
  STATIC_CHECK(std::is_void_v<type>);
}

TEST_CASE("homogenized_type with compatible but different types (strict fallback)",
          "[homogenized_type]")
{
  using type = deferred::homogenized_type_t<int, double>;
  // Strict behavior: must be a variant.
  STATIC_CHECK(std::is_same_v<type, std::variant<int, double>>);
}

TEST_CASE("homogenized_type with void", "[homogenized_type]")
{
  using type = deferred::homogenized_type_t<void, void>;
  STATIC_CHECK(std::is_void_v<type>);
}

TEST_CASE("homogenized_type mixed with void (variant fallback)", "[homogenized_type]")
{
  using type = deferred::homogenized_type_t<int, void>;
  STATIC_CHECK(std::is_same_v<type, std::variant<int, std::monostate>>);
}

TEST_CASE("homogenized_type incompatible types (variant fallback)", "[homogenized_type]")
{
  using type = deferred::homogenized_type_t<int, std::string>;
  STATIC_CHECK(std::is_same_v<type, std::variant<int, std::string>>);
}

TEST_CASE("homogenized_type unique types deduplication", "[homogenized_type]")
{
  using type = deferred::homogenized_type_t<int, std::string, int, std::string>;
  STATIC_CHECK(std::is_same_v<type, std::variant<int, std::string>>);
}

TEST_CASE("homogenized_type complex mix", "[homogenized_type]")
{
  using type = deferred::homogenized_type_t<int, double, std::string, void>;
  STATIC_CHECK(std::is_same_v<type, std::variant<int, double, std::string, std::monostate>>);
}
