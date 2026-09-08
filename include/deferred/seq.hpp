// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_SEQ_HPP
#define DEFERRED_SEQ_HPP

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/is_nothrow_visitable.hpp"
#include "detail/visit_children.hpp"
#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

namespace detail {

/**
 * @brief Checks whether evaluating every expression of a sequence cannot throw.
 * @tparam ExpressionReferences Reference types the sequence evaluates through.
 * @return @c true when no evaluation can throw.
 */
template<typename... ExpressionReferences>
consteval bool sequence_is_nothrow()
{
  return (noexcept(evaluate(std::declval<ExpressionReferences>())) && ...);
}

} // namespace detail

/**
 * @brief Deferred sequence that evaluates its expressions left to right.
 *
 * Every expression is evaluated in order and the result of the last one is returned,
 * as the built-in comma operator does. This is what lets a construct that takes a
 * single expression -- a loop body, a branch, a @c for_ clause -- do more than one
 * thing without a lambda.
 *
 * @tparam Expressions Types of the sequenced expressions.
 */
template<Deferred... Expressions>
  requires(sizeof...(Expressions) > 0)
class seq_expression
{
public:
  using subexpression_types = std::tuple<Expressions...>;

private:
  [[no_unique_address]] subexpression_types m_expressions;

  static constexpr std::size_t last_index = sizeof...(Expressions) - 1;

  /**
   * @brief Evaluates every expression before the last for effect, then yields the last.
   *
   * The fold is expanded over a pack, so evaluation is left to right. Each discarded
   * result is cast to @c void so that a result type with its own @c operator, cannot
   * take part.
   *
   * @tparam Self Type of the sequence being evaluated.
   * @tparam I Indices of the expressions to evaluate for effect.
   * @param self Sequence being evaluated.
   * @return The result of the last expression.
   */
  template<typename Self, std::size_t... I>
  static constexpr decltype(auto) evaluate_impl(Self& self, std::index_sequence<I...>)
  {
    (static_cast<void>(evaluate(std::get<I>(self.m_expressions))), ...);
    return evaluate(std::get<last_index>(self.m_expressions));
  }

public:
  /**
   * @brief Constructs a seq_expression.
   * @tparam Ex Types of the sequenced expressions.
   * @param ex Expressions to evaluate in order.
   */
  template<typename... Ex>
  constexpr explicit seq_expression(Ex&&... ex) noexcept(
    std::is_nothrow_constructible_v<subexpression_types, Ex&&...>) :
    m_expressions(std::forward<Ex>(ex)...)
  { }

  /**
   * @brief Evaluates every expression in order.
   * @return The result of the last expression.
   */
  constexpr decltype(auto)
  operator()() const noexcept(detail::sequence_is_nothrow<Expressions const&...>())
  {
    return evaluate_impl(*this, std::make_index_sequence<last_index>{});
  }

  /// @copydoc seq_expression::operator()() const
  constexpr decltype(auto) operator()() noexcept(detail::sequence_is_nothrow<Expressions&...>())
  {
    return evaluate_impl(*this, std::make_index_sequence<last_index>{});
  }

  /**
   * @brief Visits the sequence with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, seq_expression, subexpression_types>)
  {
    v(*this, nesting);
    detail::visit_children(m_expressions, v, nesting + 1);
  }
};

/**
 * @brief Creates a @c deferred sequence of @p expressions.
 *
 * Example:
 * @code
 * auto ex = while_(n != 0).do_(seq(counter, n -= 1));
 * @endcode
 *
 * @tparam Expressions Types of the expressions to sequence.
 * @param expressions Expressions to evaluate in order.
 * @return A @ref seq_expression evaluating each in turn and yielding the last.
 */
template<typename... Expressions>
  requires(sizeof...(Expressions) > 0)
[[nodiscard]] constexpr auto seq(Expressions&&... expressions) noexcept(
  std::is_nothrow_constructible_v<seq_expression<make_deferred_t<Expressions>...>, Expressions&&...>)
{
  return seq_expression<make_deferred_t<Expressions>...>(std::forward<Expressions>(expressions)...);
}

} // namespace deferred

#endif
