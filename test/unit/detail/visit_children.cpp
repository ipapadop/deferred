// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <tuple>
#include <vector>

#include "deferred/detail/visit_children.hpp"

namespace {

struct recording_child
{
  int id;

  template<typename Visitor>
  constexpr void visit(Visitor& v, std::size_t nesting) const
  {
    v(*this, nesting);
  }
};

struct recorder
{
  std::vector<std::pair<int, std::size_t>>* seen;

  void operator()(recording_child const& child, std::size_t nesting) const
  {
    seen->emplace_back(child.id, nesting);
  }
};

} // namespace

TEST_CASE("visit_children visits nothing for an empty tuple", "[visit-children]")
{
  std::vector<std::pair<int, std::size_t>> seen;
  recorder v{&seen};

  deferred::detail::visit_children(std::tuple<>{}, v, 1);

  CHECK(seen.empty());
}

TEST_CASE("visit_children visits every child in order", "[visit-children]")
{
  std::vector<std::pair<int, std::size_t>> seen;
  recorder v{&seen};
  auto children = std::make_tuple(recording_child{1}, recording_child{2}, recording_child{3});

  deferred::detail::visit_children(children, v, 4);

  REQUIRE(seen.size() == 3);
  CHECK(seen[0].first == 1);
  CHECK(seen[1].first == 2);
  CHECK(seen[2].first == 3);
}

TEST_CASE("visit_children reports the nesting level it is given", "[visit-children]")
{
  std::vector<std::pair<int, std::size_t>> seen;
  recorder v{&seen};
  auto children = std::make_tuple(recording_child{1}, recording_child{2});

  deferred::detail::visit_children(children, v, 7);

  REQUIRE(seen.size() == 2);
  CHECK(seen[0].second == 7);
  CHECK(seen[1].second == 7);
}

TEST_CASE("visit_children visits each child exactly once", "[visit-children]")
{
  std::vector<std::pair<int, std::size_t>> seen;
  recorder v{&seen};
  auto children = std::make_tuple(recording_child{1}, recording_child{1});

  deferred::detail::visit_children(children, v, 0);

  CHECK(seen.size() == 2);
}
