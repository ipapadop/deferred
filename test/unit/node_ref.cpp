// SPDX-FileCopyrightText: 2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "deferred/constant.hpp"
#include "deferred/invoke.hpp"
#include "deferred/node_ref.hpp"
#include "deferred/operators.hpp"
#include "deferred/type_traits/expression_visitor.hpp"
#include "deferred/type_traits/is_constant_expression.hpp"
#include "deferred/type_traits/is_deferred.hpp"
#include "deferred/variable.hpp"

namespace {

using V   = deferred::variable<int>;
using R   = deferred::variable_ref<int>;
using CR  = deferred::variable_cref<int>;
using Sum = deferred::expression_<std::plus<>, R, deferred::constant<int>>;

/// @brief Records the address of every variable it is called with.
struct variable_address_visitor
{
  void const* seen  = nullptr;
  std::size_t calls = 0;

  template<typename T>
  void operator()(deferred::variable<T> const& v, std::size_t)
  {
    seen = &v;
    ++calls;
  }

  template<typename T>
  void operator()(T const&, std::size_t)
  {
    ++calls;
  }
};

/// @brief Accepts a variable only, so it cannot be called with a variable_ref.
struct variable_only_visitor
{
  void operator()(V const&, std::size_t) const noexcept
  { }
};

template<typename T>
concept has_get = requires(T t) { t.get(); };

} // namespace

TEST_CASE("a variable_ref is a leaf node with mutable state", "[node-ref]")
{
  static_assert(deferred::deferred_expression<R>);
  static_assert(std::is_same_v<R::node_type, V>);
  static_assert(std::is_same_v<R::subexpression_types, std::tuple<>>);
  static_assert(R::mutable_state);
  static_assert(!deferred::is_constant_expression_v<R>);

  static_assert(deferred::deferred_expression<CR>);
  static_assert(std::is_same_v<CR::node_type, V const>);
  static_assert(std::is_same_v<CR::subexpression_types, std::tuple<>>);
  static_assert(CR::mutable_state);
  static_assert(!deferred::is_constant_expression_v<CR>);
  static_assert(sizeof(CR) == sizeof(void*));
  static_assert(std::is_trivially_copy_constructible_v<CR>);
  static_assert(!std::is_copy_assignable_v<CR>);

  // It is a pointer, and copying it is trivial.
  static_assert(sizeof(R) == sizeof(void*));
  static_assert(std::is_trivially_copy_constructible_v<R>);
}

TEST_CASE("a variable_ref is built from a named variable only", "[node-ref]")
{
  static_assert(std::is_constructible_v<R, V&>);
  static_assert(std::is_nothrow_constructible_v<R, V&>);

  // A temporary would not outlive the reference.
  static_assert(!std::is_constructible_v<R, V>);
  static_assert(!std::is_constructible_v<R, V&&>);
  static_assert(!std::is_constructible_v<CR, V const&&>);

  // A const variable is referred to as const.
  static_assert(!std::is_constructible_v<R, V const&>);
  static_assert(std::is_constructible_v<CR, V const&>);
  static_assert(std::is_constructible_v<CR, V&>);

  // Never by accident.
  static_assert(!std::is_convertible_v<V&, R>);
  static_assert(!std::is_convertible_v<V const&, CR>);
  static_assert(!std::is_default_constructible_v<R>);
  static_assert(!std::is_default_constructible_v<CR>);
}

TEST_CASE("a variable_ref reads and writes the variable it refers to", "[node-ref]")
{
  V v{1};
  R r{v};

  static_assert(std::is_same_v<decltype(r()), int&>);
  static_assert(std::is_same_v<decltype(std::as_const(r)()), int&>);
  static_assert(noexcept(r()));

  CHECK(&r() == &v());

  v = 2;
  CHECK(r() == 2);

  r() = 3;
  CHECK(v() == 3);

  // get() is a named spelling of the call, as it is for a variable.
  static_assert(std::is_same_v<decltype(r.get()), decltype(r())>);
  static_assert(noexcept(r.get()));
  r.get() = 4;
  CHECK(v.get() == 4);
}

TEST_CASE("a variable_cref reads a const variable and cannot write it", "[node-ref]")
{
  V const v{4};
  CR r{v};

  static_assert(std::is_same_v<decltype(r()), int const&>);
  static_assert(std::is_same_v<decltype(r.get()), int const&>);
  static_assert(noexcept(r()));
  CHECK(&r() == &v());
  CHECK(r.get() == 4);

  // It reads a variable that is const only where the expression is built, and sees
  // writes made through the variable itself.
  V w{5};
  CR view{std::as_const(w)};
  w = 6;
  CHECK(view() == 6);
}

TEST_CASE("a variable of a const type is referred to by a variable_ref", "[node-ref]")
{
  using KV = deferred::variable<int const>;
  using KR = deferred::variable_ref<int const>;

  KV k{5};

  // The type of the value is const, which is distinct from the variable being const.
  static_assert(std::is_same_v<KR::node_type, KV>);
  static_assert(std::is_constructible_v<KR, KV&>);
  static_assert(!std::is_constructible_v<KR, V&>);
  static_assert(!std::is_constructible_v<KR, V const&>);
  static_assert(std::is_same_v<decltype(KR{k}()), int const&>);
  static_assert(
    std::is_same_v<deferred::make_deferred_t<KV const&>, deferred::variable_cref<int const>>);

  auto sum = k + deferred::constant(1);
  static_assert(
    std::is_same_v<decltype(sum), deferred::expression_<std::plus<>, KR, deferred::constant<int>>>);
  CHECK(sum() == 6);

  variable_address_visitor visitor;
  sum.visit(visitor);
  CHECK(visitor.seen == &k);
}

TEST_CASE("copies of a variable_ref refer to the same variable", "[node-ref]")
{
  V v{1};
  R r{v};
  R copy{r};

  copy() = 5;
  CHECK(r() == 5);
  CHECK(v() == 5);

  // Like every node, it is not assignable.
  static_assert(!std::is_copy_assignable_v<R>);
  static_assert(!std::is_move_assignable_v<R>);
}

TEST_CASE("an expression stores a variable_ref for a named variable", "[node-ref]")
{
  V v{1};
  V const cv{2};
  auto c = deferred::constant(10);

  static_assert(std::is_same_v<decltype(v + c), Sum>);
  static_assert(std::is_same_v<decltype(cv + c),
                               deferred::expression_<std::plus<>, CR, deferred::constant<int>>>);
  static_assert(
    std::is_same_v<decltype(++v), deferred::expression_<deferred::detail::pre_increment, R>>);
  static_assert(
    std::is_same_v<
      decltype(v = c),
      deferred::assign_expression<deferred::detail::assign_to, R, deferred::constant<int>>>);

  auto sum = v + c;
  CHECK(sum() == 11);
  v = 5;
  CHECK(sum() == 15);

  // A copy of the expression refers to the same variable.
  auto copy = sum;
  v         = 7;
  CHECK(copy() == 17);
}

TEST_CASE("a visitor is called with the variable, not the variable_ref", "[node-ref]")
{
  V v{1};
  R r{v};

  variable_address_visitor visitor;
  r.visit(visitor);
  CHECK(visitor.seen == &v);
  CHECK(visitor.calls == 1);

  // The whole tree is checked against the variable too.
  static_assert(deferred::expression_visitor<variable_only_visitor, R>);
  static_assert(deferred::expression_visitor<variable_address_visitor, Sum>);
  static_assert(!deferred::expression_visitor<variable_only_visitor, Sum>);
  static_assert(noexcept(r.visit(variable_only_visitor{})));
  static_assert(!noexcept(r.visit(visitor)));

  V const cv{2};
  CR cr{cv};
  variable_address_visitor const_visitor;
  cr.visit(const_visitor);
  CHECK(const_visitor.seen == &cv);
  static_assert(deferred::expression_visitor<variable_only_visitor, CR>);

  auto sum = v + deferred::constant(1);
  variable_address_visitor tree_visitor;
  sum.visit(tree_visitor);
  CHECK(tree_visitor.seen == &v);
  CHECK(tree_visitor.calls == 3);
}

TEST_CASE("a variable_ref works for a class type", "[node-ref]")
{
  deferred::variable<std::string> v{std::string("a")};
  deferred::variable_ref<std::string> r{v};

  r() += "b";
  CHECK(v() == "ab");
}

TEST_CASE("variable_ref and variable_cref are the node_ref forms of a variable", "[node-ref]")
{
  static_assert(std::is_same_v<deferred::variable_ref<int>, deferred::node_ref<V>>);
  static_assert(std::is_same_v<deferred::variable_cref<int>, deferred::node_ref<V const>>);
  // A variable of a const type is a different node from a const variable.
  static_assert(!std::is_same_v<deferred::variable_ref<int const>, deferred::variable_cref<int>>);
}

TEST_CASE("std::ref and std::cref make the same node_ref as naming a variable", "[node-ref]")
{
  V v{1};
  auto c = deferred::constant(2);

  static_assert(std::is_same_v<deferred::make_deferred_t<std::reference_wrapper<V>>, R>);
  static_assert(std::is_same_v<deferred::make_deferred_t<std::reference_wrapper<V const>>, CR>);
  static_assert(std::is_same_v<decltype(std::ref(v) + c), decltype(v + c)>);
  static_assert(std::is_same_v<decltype(std::cref(v) + c), decltype(std::as_const(v) + c)>);

  auto sum = std::ref(v) + c;
  v        = 5;
  CHECK(sum() == 7);
}

TEST_CASE("a node_ref refers to any node, not only a variable", "[node-ref]")
{
  auto next = deferred::invoke([count = 0]() mutable { return ++count; });
  using N   = decltype(next);

  static_assert(
    std::is_same_v<deferred::make_deferred_t<std::reference_wrapper<N>>, deferred::node_ref<N>>);
  static_assert(deferred::deferred_expression<deferred::node_ref<N>>);
  static_assert(!deferred::is_constant_expression_v<deferred::node_ref<N>>);
  static_assert(!std::is_constructible_v<deferred::node_ref<N>, N&&>);

  // Evaluating the reference evaluates the node, also from a const reference.
  deferred::node_ref<N> const r{next};
  CHECK(r() == 1);
  CHECK(r() == 2);
  CHECK(next() == 3);

  // A read-only reference evaluates the node as const.
  auto c = deferred::constant(4);
  deferred::node_ref<deferred::constant<int> const> cr{c};
  static_assert(std::is_same_v<decltype(cr()), int const&>);
  CHECK(&cr() == &c());
}

TEST_CASE("a node_ref has get() when the node it refers to has", "[node-ref]")
{
  auto next = deferred::invoke([] { return 1; });

  static_assert(has_get<R>);
  static_assert(has_get<deferred::node_ref<deferred::constant<int> const>>);
  static_assert(!has_get<deferred::node_ref<decltype(next)>>);
}

TEST_CASE("a reference_wrapper to something that is not a node stays a callable", "[node-ref]")
{
  int i         = 0;
  auto function = [&i] {
    return ++i;
  };

  using wrapped = deferred::make_deferred_t<std::reference_wrapper<decltype(function)>>;
  static_assert(
    std::is_same_v<wrapped, deferred::expression_<std::reference_wrapper<decltype(function)>>>);
}
