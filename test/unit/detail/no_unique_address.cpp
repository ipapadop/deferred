// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include "deferred/detail/no_unique_address.hpp"

namespace {

struct empty
{ };

struct with_attribute
{
  DEFERRED_NO_UNIQUE_ADDRESS empty member;
  int value;
};

struct without_attribute
{
  empty member;
  int value;
};

} // namespace

TEST_CASE("DEFERRED_NO_UNIQUE_ADDRESS lets an empty member occupy no storage",
          "[detail-no-unique-address]")
{
  // Every compiler the project supports provides one of the two spellings. If this fails
  // to compile, the compiler in use provides neither: the library is still correct, but
  // every empty subexpression now costs storage.
  STATIC_CHECK(sizeof(with_attribute) == sizeof(int));

  // The attribute is what does it: the same layout without it is larger.
  STATIC_CHECK(sizeof(without_attribute) > sizeof(int));
}
