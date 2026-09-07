// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_CONDITIONAL_HPP
#define DEFERRED_CONDITIONAL_HPP

#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/expression_reference.hpp"
#include "detail/homogenized_type.hpp"
#include "detail/is_nothrow_evaluable.hpp"
#include "detail/is_nothrow_visitable.hpp"
#include "detail/map_result.hpp"
#include "detail/visit_children.hpp"
#include "evaluate.hpp"
#include "expression.hpp"

namespace deferred {

template<typename Else, typename... Branches>
class conditional_expression;

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

  /**
   * @brief Visits the conditional branch with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, conditional_branch, subexpression_types>)
  {
    v(*this, nesting);
    condition.visit(v, nesting + 1);
    then.visit(v, nesting + 1);
  }
};

namespace detail {

/** @brief Checks whether evaluating one conditional branch cannot throw. */
template<typename Result, typename Self, typename Branch>
consteval bool conditional_branch_is_nothrow()
{
  using branch_reference    = expression_reference_t<Self, Branch>;
  using condition_reference = decltype((std::declval<branch_reference>().condition));
  using then_reference      = decltype((std::declval<branch_reference>().then));
  return noexcept(static_cast<bool>(evaluate(std::declval<condition_reference>())))
         && (std::is_void_v<Result> ? noexcept(evaluate(std::declval<then_reference>()))
                                    : evaluation_result_is_nothrow<Result, then_reference>());
}

/** @brief Checks whether evaluating every conditional path cannot throw. */
template<typename Result, typename Else, typename Self, typename... Branches>
consteval bool conditional_evaluation_is_nothrow()
{
  if constexpr (!(conditional_branch_is_nothrow<Result, Self, Branches>() && ...))
  {
    return false;
  }
  else if constexpr (std::is_same_v<Else, no_else>)
  {
    return std::is_void_v<Result> || std::is_nothrow_constructible_v<Result, std::nullopt_t>;
  }
  else
  {
    using else_reference = expression_reference_t<Self, Else>;
    return std::is_void_v<Result> ? noexcept(evaluate(std::declval<else_reference>()))
                                  : evaluation_result_is_nothrow<Result, else_reference>();
  }
}

/** @brief Stores the exception guarantee for a conditional reference type. */
template<typename Self, typename Else, typename... Branches>
struct conditional_evaluation_traits
{
  using result = typename std::remove_cvref_t<Self>::result_type;
  static constexpr bool value =
    conditional_evaluation_is_nothrow<result, Else, Self, Branches...>();
};

/** @brief Specializes evaluation traits for a const conditional expression. */
template<typename Else, typename... Branches>
struct is_nothrow_evaluable<conditional_expression<Else, Branches...> const&> :
  conditional_evaluation_traits<conditional_expression<Else, Branches...> const&, Else, Branches...>
{ };

/** @brief Specializes evaluation traits for a mutable conditional expression. */
template<typename Else, typename... Branches>
struct is_nothrow_evaluable<conditional_expression<Else, Branches...>&> :
  conditional_evaluation_traits<conditional_expression<Else, Branches...>&, Else, Branches...>
{ };

/** @brief Deduces the conditional type for a branch tuple plus any appended branches. */
template<typename Else, typename Branches, typename... Appended>
struct conditional_from_branches;

/** @brief Specializes conditional deduction for a branch tuple. */
template<typename Else, typename... Branches, typename... Appended>
struct conditional_from_branches<Else, std::tuple<Branches...>, Appended...>
{
  using type = conditional_expression<Else, Branches..., Appended...>;
};

/** @brief The conditional type for a branch tuple plus any appended branches. */
template<typename Else, typename Branches, typename... Appended>
using conditional_from_branches_t =
  typename conditional_from_branches<Else, Branches, Appended...>::type;

/** @brief Type of the branch produced from a condition and a then expression. */
template<typename Condition, typename Then>
using branch_t = conditional_branch<make_deferred_t<Condition>, make_deferred_t<Then>>;

/** @brief Appends a branch to a conditional expression's branch tuple. */
template<typename Else, typename Branches, typename C, typename T>
constexpr auto append_conditional(Branches&& branches, C&& condition, T&& then_)
{
  using branch = branch_t<C, T>;
  using result = conditional_from_branches_t<Else, std::remove_cvref_t<Branches>, branch>;
  return result(
    std::tuple_cat(std::forward<Branches>(branches),
                   std::tuple<branch>{branch{make_deferred_t<C>(std::forward<C>(condition)),
                                             make_deferred_t<T>(std::forward<T>(then_))}}));
}

/** @brief Finalizes a conditional expression with an else expression. */
template<typename Branches, typename E>
constexpr auto finalize_conditional(Branches&& branches, E&& else_branch)
{
  using else_expression = make_deferred_t<E>;
  using result = conditional_from_branches_t<else_expression, std::remove_cvref_t<Branches>>;
  if constexpr (std::is_lvalue_reference_v<Branches&&>)
  {
    return result(std::remove_cvref_t<Branches>(branches),
                  else_expression(std::forward<E>(else_branch)));
  }
  else
  {
    return result(std::forward<Branches>(branches), else_expression(std::forward<E>(else_branch)));
  }
}

} // namespace detail

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
  static constexpr result_type
  evaluate_impl(Self&& self) noexcept(detail::is_nothrow_evaluable_v<Self>)
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
  constexpr explicit conditional_expression(branches_tuple&& branches) noexcept(
    std::is_nothrow_move_constructible_v<branches_tuple>
    && std::is_nothrow_default_constructible_v<Else>)
    requires(!finalized)
    : m_branches(std::move(branches)), m_else{}
  { }

  /**
   * @brief Constructs a finalized conditional expression.
   * @param branches Tuple of branches.
   * @param else_branch Else expression.
   */
  constexpr explicit conditional_expression(branches_tuple&& branches, Else&& else_branch) noexcept(
    std::is_nothrow_move_constructible_v<branches_tuple>
    && std::is_nothrow_move_constructible_v<Else>)
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
    return detail::append_conditional<Else>(std::move(m_branches),
                                            std::forward<C>(condition),
                                            std::forward<T>(then_));
  }

  /// @copydoc else_if
  template<typename C, typename T>
  [[nodiscard]] constexpr auto else_if(C&& condition, T&& then_) const&
    requires(!finalized)
  {
    return detail::append_conditional<Else>(m_branches,
                                            std::forward<C>(condition),
                                            std::forward<T>(then_));
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
    return detail::finalize_conditional(std::move(m_branches), std::forward<E>(else_branch));
  }

  /// @copydoc else_
  template<typename E>
  [[nodiscard]] constexpr auto else_(E&& else_branch) const&
    requires(!finalized)
  {
    return detail::finalize_conditional(m_branches, std::forward<E>(else_branch));
  }

  /**
   * @brief Evaluates the conditional expression.
   * @return Result of the conditional expression.
   */
  [[nodiscard]] constexpr result_type
  operator()() const noexcept(detail::is_nothrow_evaluable_v<conditional_expression const&>)
  {
    return evaluate_impl(*this);
  }

  /// @copydoc operator()() const
  [[nodiscard]] constexpr result_type
  operator()() noexcept(detail::is_nothrow_evaluable_v<conditional_expression&>)
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
    noexcept(detail::is_nothrow_visitable_v<Visitor, conditional_expression, subexpression_types>)
  {
    v(*this, nesting);
    detail::visit_children(m_branches, v, nesting + 1);
    if constexpr (finalized)
    {
      m_else.visit(v, nesting + 1);
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
[[nodiscard]] constexpr auto if_(Condition&& condition, Then&& then_) noexcept(
  noexcept(conditional_expression<detail::no_else, detail::branch_t<Condition, Then>>(
    std::tuple<detail::branch_t<Condition, Then>>{detail::branch_t<Condition, Then>{
      make_deferred_t<Condition>(std::forward<Condition>(condition)),
      make_deferred_t<Then>(std::forward<Then>(then_))}})))
{
  using branch = detail::branch_t<Condition, Then>;
  return conditional_expression<detail::no_else, branch>(
    std::tuple<branch>{branch{make_deferred_t<Condition>(std::forward<Condition>(condition)),
                              make_deferred_t<Then>(std::forward<Then>(then_))}});
}

} // namespace deferred

#endif
