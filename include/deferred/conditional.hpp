// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_CONDITIONAL_HPP
#define DEFERRED_CONDITIONAL_HPP

#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/map_result.hpp"
#include "evaluate.hpp"
#include "expression.hpp"
#include "type_traits/homogenized_type.hpp"

namespace deferred {

namespace detail {

/**
 * @brief Tag for conditional expressions without an @c else branch.
 */
struct no_else
{ };

/**
 * @brief Deduces the base result type of a conditional expression.
 */
template<typename Else, typename... Branches>
struct conditional_base_result_deducer
{
  using type = homogenized_type_t<decltype(std::declval<typename Branches::then_type>()())...,
                                  decltype(std::declval<Else>()())>;
};

/**
 * @brief Specialization for non-finalized conditional expressions.
 */
template<typename... Branches>
struct conditional_base_result_deducer<no_else, Branches...>
{
  using type = homogenized_type_t<decltype(std::declval<typename Branches::then_type>()())...>;
};

} // namespace detail

/**
 * @brief Represents a single branch in a conditional expression.
 * @tparam Condition Type of the condition expression.
 * @tparam Then Type of the then expression.
 */
template<Deferred Condition, Deferred Then>
struct conditional_branch
{
  using condition_type      = Condition;
  using then_type           = Then;
  using subexpression_types = std::tuple<Condition, Then>;
  [[no_unique_address]] Condition condition;
  [[no_unique_address]] Then then;
};

/**
 * @brief Deferred conditional expression.
 * @tparam Else Type of the @c else branch.
 * @tparam Branches Types of the @c if and @c else_if branches.
 */
template<typename Else, typename... Branches>
class conditional_expression
{
  constexpr static inline bool finalized = !std::is_same_v<Else, detail::no_else>;

public:
  using branches_tuple = std::tuple<Branches...>;
  using subexpression_types =
    std::conditional_t<finalized, std::tuple<Branches..., Else>, branches_tuple>;

  /**
   * @brief Result type of the underlying expressions (common type or variant).
   */
  using base_result_type =
    typename detail::conditional_base_result_deducer<Else, Branches...>::type;

  /**
   * @brief Final result type of the conditional expression.
   */
  using result_type = std::conditional_t<
    finalized,
    base_result_type,
    std::conditional_t<std::is_void_v<base_result_type>, void, std::optional<base_result_type>>>;

private:
  [[no_unique_address]] branches_tuple m_branches;
  [[no_unique_address]] Else m_else;

  template<std::size_t I = 0, typename Self>
  static constexpr result_type evaluate_impl(Self&& self)
  {
    if constexpr (I < sizeof...(Branches))
    {
      auto&& branch = std::get<I>(self.m_branches);
      if (evaluate(branch.condition))
      {
        if constexpr (std::is_void_v<result_type>)
        {
          evaluate(branch.then);
          return;
        }
        else
        {
          return detail::map_result<base_result_type>([&] { return evaluate(branch.then); });
        }
      }
      return evaluate_impl<I + 1>(std::forward<Self>(self));
    }
    else
    {
      if constexpr (!finalized)
      {
        if constexpr (!std::is_void_v<result_type>)
        {
          return std::nullopt;
        }
      }
      else
      {
        return detail::map_result<result_type>([&] { return evaluate(self.m_else); });
      }
    }
  }

public:
  /**
   * @brief Constructs a non-finalized conditional expression.
   * @param branches Tuple of branches.
   */
  constexpr explicit conditional_expression(branches_tuple&& branches)
    requires(!finalized)
    : m_branches(std::move(branches)), m_else{}
  { }

  /**
   * @brief Constructs a finalized conditional expression.
   * @param branches Tuple of branches.
   * @param else_branch Else expression.
   */
  constexpr explicit conditional_expression(branches_tuple&& branches, Else&& else_branch)
    requires finalized
    : m_branches(std::move(branches)), m_else(std::forward<Else>(else_branch))
  { }

  /**
   * @brief Appends an @c else_if branch to the conditional chain.
   * @tparam C Type of the condition expression.
   * @tparam T Type of the then expression.
   * @param condition Condition expression.
   * @param then_ Then expression.
   * @return A new @ref conditional_expression with the added branch.
   */
  template<typename C, typename T>
  [[nodiscard]] constexpr auto else_if(C&& condition, T&& then_) &&
    requires(!finalized)
  {
    using cond_type   = make_deferred_t<C>;
    using then_type   = make_deferred_t<T>;
    using branch_type = conditional_branch<cond_type, then_type>;

    return conditional_expression<Else, Branches..., branch_type>(
      std::tuple_cat(std::move(m_branches),
                     std::tuple<branch_type>{branch_type{cond_type(std::forward<C>(condition)),
                                                         then_type(std::forward<T>(then_))}}));
  }

  /// @copydoc else_if
  template<typename C, typename T>
  [[nodiscard]] constexpr auto else_if(C&& condition, T&& then_) const&
    requires(!finalized)
  {
    using cond_type   = make_deferred_t<C>;
    using then_type   = make_deferred_t<T>;
    using branch_type = conditional_branch<cond_type, then_type>;

    return conditional_expression<Else, Branches..., branch_type>(
      std::tuple_cat(m_branches,
                     std::tuple<branch_type>{branch_type{cond_type(std::forward<C>(condition)),
                                                         then_type(std::forward<T>(then_))}}));
  }

  /**
   * @brief Finalizes the conditional chain with an @c else branch.
   * @tparam E Type of the else expression.
   * @param else_branch Else expression.
   * @return A finalized @ref conditional_expression.
   */
  template<typename E>
  [[nodiscard]] constexpr auto else_(E&& else_branch) &&
    requires(!finalized)
  {
    using else_expr = make_deferred_t<E>;
    return conditional_expression<else_expr, Branches...>(std::move(m_branches),
                                                          else_expr(std::forward<E>(else_branch)));
  }

  /// @copydoc else_
  template<typename E>
  [[nodiscard]] constexpr auto else_(E&& else_branch) const&
    requires(!finalized)
  {
    using else_expr = make_deferred_t<E>;
    return conditional_expression<else_expr, Branches...>(m_branches,
                                                          else_expr(std::forward<E>(else_branch)));
  }

  /**
   * @brief Evaluates the conditional expression.
   * @return Result of the conditional expression.
   */
  [[nodiscard]] constexpr result_type operator()() const
  {
    return evaluate_impl(*this);
  }

  /// @copydoc operator()() const
  [[nodiscard]] constexpr result_type operator()()
  {
    return evaluate_impl(*this);
  }

  /**
   * @brief Visits the expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
  {
    std::forward<Visitor>(v)(*this, nesting);
    std::apply(
      [&](auto const&... branch) {
        ((branch.condition.visit(std::forward<Visitor>(v), nesting + 1),
          branch.then.visit(std::forward<Visitor>(v), nesting + 1)),
         ...);
      },
      m_branches);
    if constexpr (finalized)
    {
      m_else.visit(std::forward<Visitor>(v), nesting + 1);
    }
  }
};

/**
 * @brief Starts a deferred conditional chain.
 *
 * Example:
 * @code
 * auto ex = if_(cond1, then1)
 *            .else_if(cond2, then2)
 *            .else_(else_expr);
 * @endcode
 *
 * @tparam Condition Type of the condition expression.
 * @tparam Then Type of the then expression.
 * @param condition Condition expression.
 * @param then_ Then expression.
 * @return An @ref conditional_expression builder.
 */
template<typename Condition, typename Then>
[[nodiscard]] constexpr auto if_(Condition&& condition, Then&& then_)
{
  using cond_type   = make_deferred_t<Condition>;
  using then_type   = make_deferred_t<Then>;
  using branch_type = conditional_branch<cond_type, then_type>;
  return conditional_expression<detail::no_else, branch_type>(
    std::tuple<branch_type>{branch_type{cond_type(std::forward<Condition>(condition)),
                                        then_type(std::forward<Then>(then_))}});
}

} // namespace deferred

#endif
