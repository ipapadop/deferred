/** @file */
/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_EXPRESSION_HPP
#define DEFERRED_EXPRESSION_HPP

#include <tuple>
#include <type_traits>
#include <utility>

#include "apply.hpp"
#include "constant.hpp"
#include "make_function_object.hpp"
#include "tuple.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

/**
 * Deferred expression that is composed of an operator @p Operator applied to
 * subexpressions @p Expressions....
 */
template<typename Operator, typename... Expressions>
class expression_ : private Operator, private std::tuple<Expressions...>
{
public:
  using operator_type       = Operator;
  using expression_types    = std::tuple<Expressions...>;
  using subexpression_types = std::tuple<Operator, Expressions...>;

  template<typename Op,
           typename... Ex,
           std::enable_if_t<!std::is_same_v<Op, expression_>>* = nullptr>
  constexpr explicit expression_(Op&& op, Ex&&... ex) :
    Operator(std::forward<Op>(op)), std::tuple<Expressions...>(
                                      std::forward<Ex>(ex)...)
  { }

  expression_(expression_ const&) = default;
  expression_(expression_&&)      = default;

  ~expression_() = default;

  expression_& operator=(expression_ const&) = delete;
  expression_& operator=(expression_&&) = delete;

  constexpr decltype(auto) operator()() const
  {
    return deferred::apply(static_cast<Operator const&>(*this),
                           static_cast<expression_types const&>(*this));
  }

  constexpr decltype(auto) operator()()
  {
    return deferred::apply(static_cast<Operator&>(*this),
                           static_cast<expression_types&>(*this));
  }

  constexpr operator_type const& operator_() const noexcept
  {
    return static_cast<Operator const&>(*this);
  }

  constexpr decltype(auto) subexpressions() const noexcept
  {
    return static_cast<expression_types const&>(*this);
  }

  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    for_each(static_cast<expression_types const&>(*this),
             [&v, nesting](auto& t) {
               t.visit(std::forward<Visitor>(v), nesting + 1);
             });
  }
};

namespace detail {

template<typename T, typename = std::void_t<>>
struct make_deferred
{
  using type = constant_<T>;
};

template<typename T>
struct make_deferred<T, std::enable_if_t<std::is_invocable_v<T>>>
{
  using type =
    expression_<std::decay_t<decltype(make_function_object(std::declval<T>()))>>;
};

} // namespace detail

/**
 * Transforms @p T into a @c deferred type.
 *
 * - If @p T is already a @c deferred type, it does not change.
 * - If @p T is a callable type, it is transformed to an @ref expression_.
 * - If @p T is not a callable type, it is transformed to an @ref constant_.
 */
template<typename T>
using make_deferred_t =
  std::conditional_t<is_deferred_v<std::decay_t<T>>,
                     T,
                     typename detail::make_deferred<std::decay_t<T>>::type>;

} // namespace deferred

#endif
