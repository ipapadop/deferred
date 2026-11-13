// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "deferred/detail/forwarded_pair.hpp"

namespace {

using deferred::detail::forwarded_pair;

/// @brief Built from the two members of a forwarded_pair, as a branch or a case is.
struct two_strings
{
  std::string first;
  std::string second;

  template<typename First, typename Second>
  explicit two_strings(forwarded_pair<First, Second> parts) :
    first(std::forward<First>(parts.first)), second(std::forward<Second>(parts.second))
  { }
};

} // namespace

TEST_CASE("forwarded_pair holds references of the categories it is given", "[forwarded-pair]")
{
  static_assert(std::is_aggregate_v<forwarded_pair<int, int>>);
  static_assert(std::is_same_v<decltype(forwarded_pair<int&, int>::first), int&>);
  static_assert(std::is_same_v<decltype(forwarded_pair<int&, int>::second), int&&>);
  static_assert(std::is_same_v<decltype(forwarded_pair<int const&, int>::first), int const&>);
  static_assert(sizeof(forwarded_pair<int&, int>) == 2 * sizeof(void*));
}

TEST_CASE("forwarded_pair forwards each member with its own category", "[forwarded-pair]")
{
  std::string kept(40, 'k');
  std::string given(40, 'g');

  two_strings built(forwarded_pair<std::string&, std::string>{kept, std::move(given)});

  CHECK(built.first == std::string(40, 'k'));
  CHECK(built.second == std::string(40, 'g'));
  CHECK(kept == std::string(40, 'k'));
  CHECK(given.empty());
}

TEST_CASE("a tuple constructs an element in place from a forwarded_pair", "[forwarded-pair]")
{
  std::string kept(40, 'k');

  std::tuple<two_strings, int> t(forwarded_pair<std::string&, std::string>{kept, std::string("b")},
                                 1);

  CHECK(std::get<0>(t).first == kept);
  CHECK(std::get<0>(t).second == "b");
}
