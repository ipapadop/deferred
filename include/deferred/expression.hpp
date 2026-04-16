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
#include "type_traits/is_deferred.hpp"

namespace deferred {

/**
 * Deferred expression that is composed of an operator @p Operator applied to
 * subexpressions @p Expressions....
 *
 * @tparam Operator The type of the operator.
 * @tparam Expressions The types of the subexpressions.
 */
template<typename Operator, typename... Expressions>
class expression_
{
  [[no_unique_address]] Operator m_op;
  [[no_unique_address]] std::tuple<Expressions...> m_expressions;

public:
  using operator_type       = Operator;
  using expression_types    = std::tuple<Expressions...>;
  using subexpression_types = std::tuple<Operator, Expressions...>;

  /**
   * @brief Constructs an expression.
   * @tparam Op The type of the operator.
   * @tparam Ex The types of the subexpressions.
   * @param op The operator.
   * @param ex The subexpressions.
   */
  template<typename Op, typename... Ex>
    requires(!std::is_same_v<std::remove_cvref_t<Op>, expression_>)
  constexpr explicit expression_(Op&& op, Ex&&... ex) :
    m_op(std::forward<Op>(op)), m_expressions(std::forward<Ex>(ex)...)
  { }

  expression_(expression_ const&) = default;
  expression_(expression_&&)      = default;

  ~expression_() = default;

  expression_& operator=(expression_ const&) = delete;
  expression_& operator=(expression_&&)      = delete;

  constexpr decltype(auto) operator()() const
  {
    return deferred::apply(m_op, m_expressions);
  }

  /// @copydoc operator()() const
  constexpr decltype(auto) operator()()
  {
    return deferred::apply(m_op, m_expressions);
  }

  constexpr operator_type const& operator_() const noexcept
  {
    return m_op;
  }

  constexpr expression_types const& subexpressions() const noexcept
  {
    return m_expressions;
  }

  /**
   * @brief Visits the expression with a visitor.
   * @tparam Visitor The type of the visitor.
   * @param v The visitor.
   * @param nesting The nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    std::apply([&v, nesting](
                 auto const&... args) { (args.visit(std::forward<Visitor>(v), nesting + 1), ...); },
               m_expressions);
  }
};

namespace detail {

template<typename T>
struct make_deferred
{
  using type = constant_<T>;
};

template<typename T>
  requires std::is_invocable_v<T>
struct make_deferred<T>
{
  using type = expression_<std::decay_t<decltype(make_function_object(std::declval<T>()))>>;
};

} // namespace detail

/**
 * Transforms @p T into a @c deferred type.
 *
 * - If @p T is already a @c deferred type, it does not change.
 * - If @p T is a callable type, it is transformed to an @ref expression_.
 * - If @p T is not a callable type, it is transformed to an @ref constant_.
 *
 * @tparam T The type to transform.
 */
template<typename T>
using make_deferred_t =
  std::conditional_t<Deferred<T>, T, typename detail::make_deferred<std::decay_t<T>>::type>;

} // namespace deferred

#endif
