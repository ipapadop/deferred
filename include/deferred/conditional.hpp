// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_CONDITIONAL_HPP
#define DEFERRED_CONDITIONAL_HPP

#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "concepts.hpp"
#include "detail/expression_reference.hpp"
#include "detail/forwarded_pair.hpp"
#include "detail/homogenized_type.hpp"
#include "detail/is_nothrow_evaluable.hpp"
#include "detail/is_nothrow_visitable.hpp"
#include "detail/map_result.hpp"
#include "detail/no_match_is_nothrow.hpp"
#include "detail/no_unique_address.hpp"
#include "detail/tuple_references.hpp"
#include "detail/unmatched_result.hpp"
#include "detail/visit_children.hpp"
#include "evaluate.hpp"
#include "expression.hpp"
#include "type_traits/expression_visitor.hpp"

namespace deferred {

template<typename Else, deferred_expression... Branches>
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
template<deferred_expression Condition, deferred_expression Then>
struct conditional_branch
{
  using then_type           = Then;
  using subexpression_types = std::tuple<Condition, Then>;
  DEFERRED_NO_UNIQUE_ADDRESS Condition condition;
  DEFERRED_NO_UNIQUE_ADDRESS Then then;

  /**
   * @brief Constructs a branch in place from the arguments of its two expressions.
   *
   * The arguments arrive as one @ref detail::forwarded_pair , so that a @c std::tuple of
   * branches can construct this one directly, without a temporary branch to move from.
   *
   * @tparam C Type of the condition argument.
   * @tparam T Type of the then argument.
   * @param parts The condition and then arguments.
   */
  template<typename C, typename T>
  constexpr explicit conditional_branch(detail::forwarded_pair<C, T> parts) noexcept(
    std::is_nothrow_constructible_v<Condition, C&&> && std::is_nothrow_constructible_v<Then, T&&>) :
    condition(std::forward<C>(parts.first)), then(std::forward<T>(parts.second))
  { }

  /**
   * @brief Returns references to the subexpressions.
   * @return A @c std::tuple of references, in @c subexpression_types order.
   */
  [[nodiscard]] constexpr auto subexpressions() const noexcept
  {
    return std::forward_as_tuple(condition, then);
  }

  /**
   * @brief Visits the conditional branch with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
    requires expression_visitor<Visitor, conditional_branch>
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
  return noexcept(static_cast<bool>(evaluate_ref(std::declval<condition_reference>())))
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
template<typename Branches, typename Branch, typename Parts, std::size_t... I>
consteval bool carried_branches_are_nothrow(std::index_sequence<I...>)
{
  using result = conditional_from_branches_t<no_else, std::remove_cvref_t<Branches>, Branch>;
  return std::is_nothrow_constructible_v<typename result::branch_types,
                                         decltype(std::get<I>(std::declval<Branches>()))...,
                                         Parts&&>;
}

/** @brief Whether appending a branch to a conditional expression cannot throw. */
template<typename Else, typename Branches, typename C, typename T, typename E>
consteval bool append_conditional_is_nothrow()
{
  using branch = conditional_branch<make_deferred_t<C>, make_deferred_t<T>>;
  return std::is_nothrow_constructible_v<Else, E&&>
         && carried_branches_are_nothrow<Branches, branch, forwarded_pair<C, T>>(
           std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Branches>>>{});
}

/**
 * @brief Appends a branch to a conditional expression's branch tuple.
 *
 * The @c else expression, or @ref no_else, is carried over unchanged, so the new branch
 * is checked after the branches already present and before the @c else.
 */
template<typename Else, typename Branches, typename C, typename T, typename E>
constexpr auto append_conditional(Branches&& branches, C&& condition, T&& then_, E&& else_) //
  noexcept(append_conditional_is_nothrow<Else, Branches, C, T, E>())
{
  using branch = conditional_branch<make_deferred_t<C>, make_deferred_t<T>>;
  using result = conditional_from_branches_t<Else, std::remove_cvref_t<Branches>, branch>;
  return result(std::forward<Branches>(branches),
                forwarded_pair<C, T>{std::forward<C>(condition), std::forward<T>(then_)},
                std::forward<E>(else_),
                std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Branches>>>{});
}

/** @brief Whether finalizing a conditional expression with an else expression cannot throw. */
template<typename Branches, typename E>
consteval bool finalize_conditional_is_nothrow()
{
  using else_expression = make_deferred_t<E>;
  using branches_type   = std::remove_cvref_t<Branches>;
  using result          = conditional_from_branches_t<else_expression, branches_type>;
  return std::is_nothrow_constructible_v<result, Branches&&, E&&>;
}

/** @brief Finalizes a conditional expression with an else expression. */
template<typename Branches, typename E>
constexpr auto finalize_conditional(Branches&& branches, E&& else_branch) //
  noexcept(finalize_conditional_is_nothrow<Branches, E>())
{
  using else_expression = make_deferred_t<E>;
  using result = conditional_from_branches_t<else_expression, std::remove_cvref_t<Branches>>;
  return result(std::forward<Branches>(branches), std::forward<E>(else_branch));
}

} // namespace detail

/**
 * @brief Conditional chain waiting for the @c then expression of its last condition.
 * @tparam Branches Tuple of the branches accumulated so far.
 * @tparam Condition Type of the pending condition expression.
 * @tparam Else Type of the @c else branch carried over, or @ref detail::no_else.
 */
template<typename Branches, deferred_expression Condition, typename Else = detail::no_else>
class conditional_builder
{
  DEFERRED_NO_UNIQUE_ADDRESS Branches m_branches;
  DEFERRED_NO_UNIQUE_ADDRESS Condition m_condition;
  DEFERRED_NO_UNIQUE_ADDRESS Else m_else;

public:
  /**
   * @brief Constructs a conditional_builder.
   * @tparam B Type of the branch tuple.
   * @tparam C Type of the condition expression.
   * @tparam E Type of the else expression.
   * @param branches Branches accumulated so far.
   * @param condition Condition expression awaiting a @c then expression.
   * @param else_branch Else expression carried over, or @ref detail::no_else.
   */
  template<typename B, typename C, typename E>
  constexpr explicit conditional_builder(B&& branches, C&& condition, E&& else_branch) noexcept(
    std::is_nothrow_constructible_v<Branches, B&&>
    && std::is_nothrow_constructible_v<Condition, C&&>
    && std::is_nothrow_constructible_v<Else, E&&>) :
    m_branches(std::forward<B>(branches)), m_condition(std::forward<C>(condition)),
    m_else(std::forward<E>(else_branch))
  { }

  /**
   * @brief Completes the pending branch with its @c then expression.
   * @tparam T Type of the then expression.
   * @param then_ Then expression.
   * @return A @ref conditional_expression with the completed branch appended.
   */
  template<typename T>
  [[nodiscard]] constexpr auto then_(T&& then_) && //
    noexcept(detail::append_conditional_is_nothrow<Else, Branches, Condition, T, Else>())
  {
    return detail::append_conditional<Else>(std::move(m_branches),
                                            std::forward<Condition>(m_condition),
                                            std::forward<T>(then_),
                                            std::forward<Else>(m_else));
  }

  /// @copydoc then_
  template<typename T>
  [[nodiscard]] constexpr auto then_(T&& then_) const& //
    noexcept(
      detail::
        append_conditional_is_nothrow<Else, Branches const&, Condition const&, T, Else const&>())
  {
    return detail::append_conditional<Else>(m_branches, m_condition, std::forward<T>(then_), m_else);
  }
};

/**
 * @brief Deferred conditional expression.
 * @tparam Else Type of the @c else branch.
 * @tparam Branches Types of the @c if and @c else_if branches.
 */
template<typename Else, deferred_expression... Branches>
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
  DEFERRED_NO_UNIQUE_ADDRESS branch_types m_branches;
  DEFERRED_NO_UNIQUE_ADDRESS Else m_else;

  template<typename C>
  using builder_type = conditional_builder<branch_types, make_deferred_t<C>, Else>;

  template<std::size_t I = 0, typename Self>
  static constexpr result_type
  evaluate_impl(Self&& self) noexcept(detail::is_nothrow_evaluable_v<Self>)
  {
    if constexpr (I < sizeof...(Branches))
    {
      auto&& branch = std::get<I>(self.m_branches);
      if (evaluate_ref(branch.condition))
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
   * @brief Constructs a conditional expression by appending a branch to the branches of
   * another.
   *
   * The branches already present are carried over element by element rather than
   * through @c std::tuple_cat, so each is constructed exactly once and the exception
   * guarantee of the whole operation is computable.
   *
   * @tparam Accumulated Type of the branch tuple to extend.
   * @tparam Branch Type of the branch to append.
   * @tparam E Type of the else expression.
   * @tparam I Indices of the branches to carry over.
   * @param branches Branches accumulated so far.
   * @param appended Branch to append after them.
   * @param else_branch Else expression carried over, or @ref detail::no_else.
   */
  template<typename Accumulated, typename Branch, typename E, std::size_t... I>
  constexpr explicit conditional_expression(Accumulated&& branches,
                                            Branch&& appended,
                                            E&& else_branch,
                                            std::index_sequence<I...>) //
    noexcept(std::is_nothrow_constructible_v<branch_types,
                                             decltype(std::get<I>(std::declval<Accumulated>()))...,
                                             Branch&&>
             && std::is_nothrow_constructible_v<Else, E&&>) :
    m_branches(std::get<I>(std::forward<Accumulated>(branches))..., std::forward<Branch>(appended)),
    m_else(std::forward<E>(else_branch))
  { }

  /**
   * @brief Constructs a finalized conditional expression.
   * @tparam B Type of the tuple of branches.
   * @tparam E Type of the else expression argument.
   * @param branches Tuple of branches.
   * @param else_branch Else expression, or the argument it is constructed from.
   */
  template<typename B, typename E>
    requires finalized
  constexpr explicit conditional_expression(B&& branches, E&& else_branch) noexcept(
    std::is_nothrow_constructible_v<branch_types, B&&>
    && std::is_nothrow_constructible_v<Else, E&&>) :
    m_branches(std::forward<B>(branches)), m_else(std::forward<E>(else_branch))
  { }

  /**
   * @brief Starts an @c else_if branch of the conditional chain.
   *
   * A new branch is checked after the branches already present and before the @c else
   * branch, so it may be added to a conditional expression that already has one.
   *
   * @tparam C Type of the condition expression.
   * @param condition Condition expression.
   * @return A @ref conditional_builder awaiting the @c then expression.
   */
  template<condition C>
  [[nodiscard]] constexpr auto else_if_(C&& condition) && //
    noexcept(std::is_nothrow_constructible_v<builder_type<C>, branch_types&&, C&&, Else&&>)
  {
    return builder_type<C>(std::move(m_branches),
                           std::forward<C>(condition),
                           std::forward<Else>(m_else));
  }

  /// @copydoc else_if_
  template<condition C>
  [[nodiscard]] constexpr auto else_if_(C&& condition) const& //
    noexcept(std::is_nothrow_constructible_v<builder_type<C>, branch_types const&, C&&, Else const&>)
  {
    return builder_type<C>(m_branches, std::forward<C>(condition), m_else);
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
  constexpr result_type
  operator()() const noexcept(detail::is_nothrow_evaluable_v<conditional_expression const&>)
  {
    return evaluate_impl(*this);
  }

  /// @copydoc operator()() const
  constexpr result_type
  operator()() noexcept(detail::is_nothrow_evaluable_v<conditional_expression&>)
  {
    return evaluate_impl(*this);
  }

  /**
   * @brief Evaluates the expression.
   *
   * A named spelling of <tt>operator()</tt>.
   *
   * @return The result of calling the expression.
   */
  constexpr decltype(auto) eval() const noexcept(noexcept((*this)()))
  {
    return (*this)();
  }

  /// @copydoc conditional_expression::eval() const
  constexpr decltype(auto) eval() noexcept(noexcept((*this)()))
  {
    return (*this)();
  }

  /**
   * @brief Returns references to the subexpressions.
   * @return A @c std::tuple of references, in @c subexpression_types order.
   */
  [[nodiscard]] constexpr auto subexpressions() const noexcept
  {
    if constexpr (finalized)
    {
      return std::tuple_cat(detail::tuple_references(m_branches), std::forward_as_tuple(m_else));
    }
    else
    {
      return detail::tuple_references(m_branches);
    }
  }

  /**
   * @brief Visits the expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v Visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
    requires expression_visitor<Visitor, conditional_expression>
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
template<condition Condition>
[[nodiscard]] constexpr auto if_(Condition&& condition) noexcept(
  std::is_nothrow_constructible_v<make_deferred_t<Condition>, Condition&&>)
{
  return conditional_builder<std::tuple<>, make_deferred_t<Condition>>(
    std::tuple<>{},
    std::forward<Condition>(condition),
    detail::no_else{});
}

} // namespace deferred

#endif
