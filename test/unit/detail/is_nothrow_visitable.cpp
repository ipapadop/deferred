// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <tuple>

#include "deferred/detail/is_nothrow_visitable.hpp"

namespace {

struct node
{ };

struct nothrow_child
{
  template<typename Visitor>
  constexpr void visit(Visitor&&, std::size_t = 0) const noexcept
  { }
};

struct throwing_child
{
  template<typename Visitor>
  constexpr void visit(Visitor&&, std::size_t = 0) const noexcept(false)
  { }
};

struct nothrow_visitor
{
  template<typename Node>
  constexpr void operator()(Node const&, std::size_t) const noexcept
  { }
};

struct throwing_visitor
{
  template<typename Node>
  constexpr void operator()(Node const&, std::size_t) const noexcept(false)
  { }
};

template<typename Visitor, typename... Children>
inline constexpr bool visitable_v =
  deferred::detail::is_nothrow_visitable_v<Visitor, node, std::tuple<Children...>>;

} // namespace

TEST_CASE("is_nothrow_visitable with no children tracks the visitor", "[is-nothrow-visitable]")
{
  STATIC_CHECK(visitable_v<nothrow_visitor>);
  STATIC_CHECK(!visitable_v<throwing_visitor>);
}

TEST_CASE("is_nothrow_visitable requires every child to be nothrow", "[is-nothrow-visitable]")
{
  STATIC_CHECK(visitable_v<nothrow_visitor, nothrow_child>);
  STATIC_CHECK(visitable_v<nothrow_visitor, nothrow_child, nothrow_child>);
  STATIC_CHECK(!visitable_v<nothrow_visitor, throwing_child>);
}

TEST_CASE("is_nothrow_visitable fails if any single child throws", "[is-nothrow-visitable]")
{
  STATIC_CHECK(!visitable_v<nothrow_visitor, nothrow_child, throwing_child>);
  STATIC_CHECK(!visitable_v<nothrow_visitor, throwing_child, nothrow_child>);
  STATIC_CHECK(!visitable_v<nothrow_visitor, nothrow_child, nothrow_child, throwing_child>);
}

TEST_CASE("is_nothrow_visitable fails for a throwing visitor regardless of children",
          "[is-nothrow-visitable]")
{
  STATIC_CHECK(!visitable_v<throwing_visitor, nothrow_child>);
  STATIC_CHECK(!visitable_v<throwing_visitor, throwing_child>);
}

TEST_CASE("is_nothrow_visitable exposes the trait as well as the variable template",
          "[is-nothrow-visitable]")
{
  using trait =
    deferred::detail::is_nothrow_visitable<nothrow_visitor, node, std::tuple<nothrow_child>>;
  STATIC_CHECK(trait::value);
}
