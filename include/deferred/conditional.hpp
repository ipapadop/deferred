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
#include "detail/no_match_is_nothrow.hpp"
#include "detail/unmatched_result.hpp"
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
struct conditional_base_result
{
  using type = homogenized_type_t<evaluated_result_t<typename Branches::then_type>...,
                                  evaluated_result_t<Else>>;
};

/**
 * @brief Specialization for non-finalized conditional expressions.
 */
template<typename... Branches>
struct conditional_base_result<no_else, Branches...>
{
  using type = homogenized_type_t<evaluated_result_t<typename Branches::then_type>...>;
};

/** @brief The base result type of a conditional expression. */
template<typename Else, typename... Branches>
using conditional_base_result_t = typename conditional_base_result<Else, Branches...>::type;

} // namespace detail

/**
 * @brief Represents a single branch in a conditional expression.
 * @tparam Condition Type of the condition expression.
 * @tparam Then Type of the then expression.
 */
template<Deferred Condition, Deferred Then>
struct conditional_branch
{
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
    return no_match_is_nothrow_v<Result>;
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

/** @brief Whether carrying a branch tuple over and appending one more branch cannot throw. */
template<typename Branches, typename Branch, std::size_t... I>
consteval bool carried_branches_are_nothrow(std::index_sequence<I...>)
{
  using result = conditional_from_branches_t<no_else, std::remove_cvref_t<Branches>, Branch>;
  return std::is_nothrow_constructible_v<typename result::branch_types,
                                         decltype(std::get<I>(std::declval<Branches>()))...,
                                         Branch&&>;
}

/** @brief Whether appending a branch to a conditional expression cannot throw. */
template<typename Branches, typename C, typename T>
consteval bool append_conditional_is_nothrow()
{
  using branch = conditional_branch<make_deferred_t<C>, make_deferred_t<T>>;
  return std::is_nothrow_constructible_v<make_deferred_t<C>, C&&>
         && std::is_nothrow_constructible_v<make_deferred_t<T>, T&&>
         && carried_branches_are_nothrow<Branches, branch>(
           std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Branches>>>{});
}

/** @brief Appends a branch to a non-finalized conditional expression's branch tuple. */
template<typename Branches, typename C, typename T>
constexpr auto append_conditional(Branches&& branches, C&& condition, T&& then_) //
  noexcept(append_conditional_is_nothrow<Branches, C, T>())
{
  using branch = conditional_branch<make_deferred_t<C>, make_deferred_t<T>>;
  using result = conditional_from_branches_t<no_else, std::remove_cvref_t<Branches>, branch>;
  return result(std::forward<Branches>(branches),
                branch{make_deferred_t<C>(std::forward<C>(condition)),
                       make_deferred_t<T>(std::forward<T>(then_))},
                std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Branches>>>{});
}

/** @brief Whether finalizing a conditional expression with an else expression cannot throw. */
template<typename Branches, typename E>
consteval bool finalize_conditional_is_nothrow()
{
  using else_expression = make_deferred_t<E>;
  using branches_type   = std::remove_cvref_t<Branches>;
  using result          = conditional_from_branches_t<else_expression, branches_type>;
  return std::is_nothrow_constructible_v<else_expression, E&&>
         && std::is_nothrow_constructible_v<branches_type, Branches&&>
         && std::is_nothrow_constructible_v<result, branches_type&&, else_expression&&>;
}

/** @brief Finalizes a conditional expression with an else expression. */
template<typename Branches, typename E>
constexpr auto finalize_conditional(Branches&& branches, E&& else_branch) //
  noexcept(finalize_conditional_is_nothrow<Branches, E>())
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
 * @brief Conditional chain waiting for the @c then expression of its last condition.
 * @tparam Branches Tuple of the branches accumulated so far.
 * @tparam Condition Type of the pending condition expression.
 */
template<typename Branches, Deferred Condition>
class conditional_builder
{
  [[no_unique_address]] Branches m_branches;
  [[no_unique_address]] Condition m_condition;

public:
  /**
   * @brief Constructs a conditional_builder.
   * @tparam B Type of the branch tuple.
   * @tparam C Type of the condition expression.
   * @param branches Branches accumulated so far.
   * @param condition Condition expression awaiting a @c then expression.
   */
  template<typename B, typename C>
  constexpr explicit conditional_builder(B&& branches, C&& condition) noexcept(
    std::is_nothrow_constructible_v<Branches, B&&>
    && std::is_nothrow_constructible_v<Condition, C&&>) :
    m_branches(std::forward<B>(branches)), m_condition(std::forward<C>(condition))
  { }

  /**
   * @brief Completes the pending branch with its @c then expression.
   * @tparam T Type of the then expression.
   * @param then_ Then expression.
   * @return A @ref conditional_expression with the completed branch appended.
   */
  template<typename T>
  [[nodiscard]] constexpr auto then_(T&& then_) && //
    noexcept(detail::append_conditional_is_nothrow<Branches, Condition, T>())
  {
    return detail::append_conditional(std::move(m_branches),
                                      std::forward<Condition>(m_condition),
                                      std::forward<T>(then_));
  }

  /// @copydoc then_
  template<typename T>
  [[nodiscard]] constexpr auto then_(T&& then_) const& //
    noexcept(std::is_nothrow_constructible_v<Condition, Condition const&>
             && detail::append_conditional_is_nothrow<Branches const&, Condition, T>())
  {
    // The branch stores the deduced type of the condition argument, so the condition is
    // passed as its stored type: a copy of an owned expression, or the same reference
    // for a referenced one. Forwarding the member itself would deduce a reference to it.
    return detail::append_conditional(m_branches, Condition(m_condition), std::forward<T>(then_));
  }
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
  using branch_types = std::tuple<Branches...>;
  using subexpression_types =
    std::conditional_t<finalized, std::tuple<Branches..., Else>, branch_types>;

  /**
   * @brief Result type of the underlying expressions (common type or variant).
   */
  using base_result_type = detail::conditional_base_result_t<Else, Branches...>;

  /**
   * @brief Final result type of the conditional expression.
   */
  using result_type = detail::unmatched_result_t<finalized, base_result_type>;

private:
  [[no_unique_address]] branch_types m_branches;
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
        return detail::map_result<base_result_type>([&] { return evaluate(self.m_else); });
      }
    }
  }

public:
  /**
   * @brief Constructs a non-finalized conditional expression by appending a branch to
   * the branches of another.
   *
   * The branches already present are carried over element by element rather than
   * through @c std::tuple_cat, so each is constructed exactly once and the exception
   * guarantee of the whole operation is computable.
   *
   * @tparam Accumulated Type of the branch tuple to extend.
   * @tparam Branch Type of the branch to append.
   * @tparam I Indices of the branches to carry over.
   * @param branches Branches accumulated so far.
   * @param appended Branch to append after them.
   */
  template<typename Accumulated, typename Branch, std::size_t... I>
  constexpr explicit conditional_expression(Accumulated&& branches,
                                            Branch&& appended,
                                            std::index_sequence<I...>) //
    noexcept(std::is_nothrow_constructible_v<branch_types,
                                             decltype(std::get<I>(std::declval<Accumulated>()))...,
                                             Branch&&>
             && std::is_nothrow_default_constructible_v<Else>)
    requires(!finalized)
    :
    m_branches(std::get<I>(std::forward<Accumulated>(branches))..., std::forward<Branch>(appended)),
    m_else{}
  { }

  /**
   * @brief Constructs a finalized conditional expression.
   * @param branches Tuple of branches.
   * @param else_branch Else expression.
   */
  constexpr explicit conditional_expression(branch_types&& branches, Else&& else_branch) noexcept(
    std::is_nothrow_move_constructible_v<branch_types>
    && std::is_nothrow_move_constructible_v<Else>)
    requires finalized
    : m_branches(std::move(branches)), m_else(std::forward<Else>(else_branch))
  { }

  /**
   * @brief Starts an @c else_if branch of the conditional chain.
   * @tparam C Type of the condition expression.
   * @param condition Condition expression.
   * @return A @ref conditional_builder awaiting the @c then expression.
   */
  template<typename C>
  [[nodiscard]] constexpr auto else_if_(C&& condition) && //
    noexcept(std::is_nothrow_constructible_v<conditional_builder<branch_types, make_deferred_t<C>>,
                                             branch_types&&,
                                             C&&>)
    requires(!finalized)
  {
    return conditional_builder<branch_types, make_deferred_t<C>>(std::move(m_branches),
                                                                 std::forward<C>(condition));
  }

  /// @copydoc else_if_
  template<typename C>
  [[nodiscard]] constexpr auto else_if_(C&& condition) const& //
    noexcept(std::is_nothrow_constructible_v<conditional_builder<branch_types, make_deferred_t<C>>,
                                             branch_types const&,
                                             C&&>)
    requires(!finalized)
  {
    return conditional_builder<branch_types, make_deferred_t<C>>(m_branches,
                                                                 std::forward<C>(condition));
  }

  /**
   * @brief Finalizes the conditional chain with an @c else branch.
   * @tparam E Type of the else expression.
   * @param else_branch Else expression.
   * @return A finalized @ref conditional_expression.
   */
  template<typename E>
  [[nodiscard]] constexpr auto else_(E&& else_branch) && //
    noexcept(detail::finalize_conditional_is_nothrow<branch_types, E>())
    requires(!finalized)
  {
    return detail::finalize_conditional(std::move(m_branches), std::forward<E>(else_branch));
  }

  /// @copydoc else_
  template<typename E>
  [[nodiscard]] constexpr auto else_(E&& else_branch) const& //
    noexcept(detail::finalize_conditional_is_nothrow<branch_types const&, E>())
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
 * auto ex = if_(cond1).then_(then1)
 *             .else_if_(cond2).then_(then2)
 *             .else_(else_expr);
 * @endcode
 *
 * @tparam Condition Type of the condition expression.
 * @param condition Condition expression.
 * @return A @ref conditional_builder awaiting the @c then expression.
 */
template<typename Condition>
[[nodiscard]] constexpr auto if_(Condition&& condition) noexcept(
  std::is_nothrow_constructible_v<make_deferred_t<Condition>, Condition&&>)
{
  return conditional_builder<std::tuple<>, make_deferred_t<Condition>>(
    std::tuple<>{},
    std::forward<Condition>(condition));
}

} // namespace deferred

#endif
