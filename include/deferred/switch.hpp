// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_SWITCH_HPP
#define DEFERRED_SWITCH_HPP

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

template<Deferred ConditionExpression, typename DefaultExpression, Deferred... CaseExpression>
class switch_expression;

namespace detail {

/**
 * @brief Tag for switch expressions without a @c default case.
 */
struct no_default
{ };

} // namespace detail

/**
 * @brief Switch default expression.
 * @tparam Expression Type of the underlying expression.
 */
template<Deferred Expression>
class default_expression
{
public:
  using subexpression_types = std::tuple<Expression>;

private:
  [[no_unique_address]] Expression m_expression;

public:
  /**
   * @brief Constructs a default_expression.
   * @tparam T Type of the arguments.
   * @param t Arguments to forward to the underlying expression.
   */
  template<typename T>
  constexpr explicit default_expression(T&& t) noexcept(
    std::is_nothrow_constructible_v<Expression, T&&>) : m_expression(std::forward<T>(t))
  { }

  [[nodiscard]] constexpr decltype(auto)
  operator()() const noexcept(noexcept(evaluate(m_expression)))
  {
    return evaluate(m_expression);
  }

  /// @copydoc default_expression::operator()() const
  [[nodiscard]] constexpr decltype(auto) operator()() noexcept(noexcept(evaluate(m_expression)))
  {
    return evaluate(m_expression);
  }

  /**
   * @brief Visits the default expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v The visitor.
   * @param nesting The nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, default_expression, subexpression_types>)
  {
    v(*this, nesting);
    m_expression.visit(v, nesting + 1);
  }
};

/**
 * @brief Switch case expression.
 * @tparam LabelExpression Type of the label expression.
 * @tparam BodyExpression Type of the body expression.
 */
template<Deferred LabelExpression, Deferred BodyExpression>
class case_expression
{
public:
  using subexpression_types = std::tuple<LabelExpression, BodyExpression>;

private:
  [[no_unique_address]] LabelExpression m_label;
  [[no_unique_address]] BodyExpression m_body;

public:
  /**
   * @brief Constructs a case_expression.
   * @tparam LabelEx Type of the label expression.
   * @tparam BodyEx Type of the body expression.
   * @param label The label expression.
   * @param body The body expression.
   */
  template<typename LabelEx, typename BodyEx>
  constexpr explicit case_expression(LabelEx&& label, BodyEx&& body) noexcept(
    std::is_nothrow_constructible_v<LabelExpression, LabelEx&&>
    && std::is_nothrow_constructible_v<BodyExpression, BodyEx&&>) :
    m_label(std::forward<LabelEx>(label)), m_body(std::forward<BodyEx>(body))
  { }

  /// @brief Compares @p T with the label expression.
  template<typename T>
  [[nodiscard]] constexpr decltype(auto)
  compare(T const& t) const noexcept(noexcept(t == evaluate(m_label)))
  {
    return t == evaluate(m_label);
  }

  /// @copydoc case_expression::compare
  template<typename T>
  [[nodiscard]] constexpr decltype(auto)
  compare(T const& t) noexcept(noexcept(t == evaluate(m_label)))
  {
    return t == evaluate(m_label);
  }

  /// @brief Returns the result of the body expression.
  [[nodiscard]] constexpr decltype(auto) operator()() const noexcept(noexcept(evaluate(m_body)))
  {
    return evaluate(m_body);
  }

  /// @copydoc case_expression::operator()() const
  [[nodiscard]] constexpr decltype(auto) operator()() noexcept(noexcept(evaluate(m_body)))
  {
    return evaluate(m_body);
  }

  /**
   * @brief Visits the case expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v The visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, case_expression, subexpression_types>)
  {
    v(*this, nesting);
    m_label.visit(v, nesting + 1);
    m_body.visit(v, nesting + 1);
  }
};

namespace detail {

/**
 * @brief Deduces the base result type of a switch expression.
 */
template<typename Default, typename... Cases>
struct switch_base_result
{
  using type = homogenized_type_t<evaluated_result_t<Default>, evaluated_result_t<Cases>...>;
};

/**
 * @brief Specialization for switch expressions without a @c default case.
 */
template<typename... Cases>
struct switch_base_result<no_default, Cases...>
{
  using type = homogenized_type_t<evaluated_result_t<Cases>...>;
};

/** @brief The base result type of a switch expression. */
template<typename Default, typename... Cases>
using switch_base_result_t = typename switch_base_result<Default, Cases...>::type;

/** @brief Checks whether evaluating one switch case or the default case cannot throw. */
template<typename Result, typename Self, typename Case>
consteval bool switch_case_is_nothrow()
{
  using case_reference = expression_reference_t<Self, Case>;
  return std::is_void_v<Result> ? noexcept(evaluate(std::declval<case_reference>()))
                                : evaluation_result_is_nothrow<Result, case_reference>();
}

/** @brief Checks whether evaluating every switch path cannot throw. */
template<typename Result,
         typename Self,
         typename ConditionExpression,
         typename DefaultExpression,
         typename... CaseExpression>
consteval bool switch_evaluation_is_nothrow()
{
  using condition_reference = expression_reference_t<Self, ConditionExpression>;
  using condition_result    = evaluated_result_t<condition_reference>;
  if constexpr (!(noexcept(evaluate(std::declval<condition_reference>()))
                  && (noexcept(static_cast<bool>(
                        std::declval<expression_reference_t<Self, CaseExpression>>().compare(
                          std::declval<condition_result const&>())))
                      && ...)
                  && (switch_case_is_nothrow<Result, Self, CaseExpression>() && ...)))
  {
    return false;
  }
  else if constexpr (std::is_same_v<DefaultExpression, no_default>)
  {
    return no_match_is_nothrow_v<Result>;
  }
  else
  {
    return switch_case_is_nothrow<Result, Self, DefaultExpression>();
  }
}

/** @brief Stores the exception guarantee for a switch reference type. */
template<typename Self,
         typename ConditionExpression,
         typename DefaultExpression,
         typename... CaseExpression>
struct switch_evaluation_traits
{
  using result                = typename std::remove_cvref_t<Self>::result_type;
  static constexpr bool value = switch_evaluation_is_nothrow<result,
                                                             Self,
                                                             ConditionExpression,
                                                             DefaultExpression,
                                                             CaseExpression...>();
};

/** @brief Specializes evaluation traits for a const switch expression. */
template<typename ConditionExpression, typename DefaultExpression, typename... CaseExpression>
struct is_nothrow_evaluable<
  switch_expression<ConditionExpression, DefaultExpression, CaseExpression...> const&> :
  switch_evaluation_traits<
    switch_expression<ConditionExpression, DefaultExpression, CaseExpression...> const&,
    ConditionExpression,
    DefaultExpression,
    CaseExpression...>
{ };

/** @brief Specializes evaluation traits for a mutable switch expression. */
template<typename ConditionExpression, typename DefaultExpression, typename... CaseExpression>
struct is_nothrow_evaluable<
  switch_expression<ConditionExpression, DefaultExpression, CaseExpression...>&> :
  switch_evaluation_traits<
    switch_expression<ConditionExpression, DefaultExpression, CaseExpression...>&,
    ConditionExpression,
    DefaultExpression,
    CaseExpression...>
{ };

} // namespace detail

/**
 * @brief Switch chain waiting for the body expression of its last case label.
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam DefaultExpression Type of the @c default case, or @ref detail::no_default.
 * @tparam Cases Tuple of the cases accumulated so far.
 * @tparam LabelExpression Type of the pending case label expression.
 */
template<Deferred ConditionExpression,
         typename DefaultExpression,
         Deferred LabelExpression,
         Deferred... Cases>
class switch_case_builder
{
  using case_types = std::tuple<Cases...>;

  [[no_unique_address]] ConditionExpression m_condition;
  [[no_unique_address]] DefaultExpression m_default;
  [[no_unique_address]] case_types m_cases;
  [[no_unique_address]] LabelExpression m_label;

public:
  /**
   * @brief Constructs a switch_case_builder.
   * @tparam Condition Type of the condition expression.
   * @tparam Default Type of the default case.
   * @tparam Cs Type of the case tuple.
   * @tparam Label Type of the label expression.
   * @param condition Condition expression.
   * @param df Default case.
   * @param cases Cases accumulated so far.
   * @param label Label expression awaiting a body expression.
   */
  template<typename Condition, typename Default, typename Cs, typename Label>
  constexpr explicit switch_case_builder(Condition&& condition,
                                         Default&& df,
                                         Cs&& cases,
                                         Label&& label) //
    noexcept(std::is_nothrow_constructible_v<ConditionExpression, Condition&&>
             && std::is_nothrow_constructible_v<DefaultExpression, Default&&>
             && std::is_nothrow_constructible_v<case_types, Cs&&>
             && std::is_nothrow_constructible_v<LabelExpression, Label&&>) :
    m_condition(std::forward<Condition>(condition)), m_default(std::forward<Default>(df)),
    m_cases(std::forward<Cs>(cases)), m_label(std::forward<Label>(label))
  { }

private:
  /** @brief Whether completing the pending case with @p Body cannot throw. */
  template<typename Condition, typename Default, typename Cs, typename Label, typename Body>
  static consteval bool then_is_nothrow()
  {
    using case_type = case_expression<LabelExpression, make_deferred_t<Body>>;
    using result = switch_expression<ConditionExpression, DefaultExpression, Cases..., case_type>;
    return std::is_nothrow_constructible_v<case_type, Label, Body&&>
           && std::is_nothrow_constructible_v<result,
                                              Condition,
                                              Default,
                                              Cs,
                                              case_type&&,
                                              std::index_sequence_for<Cases...>>;
  }

public:
  /**
   * @brief Completes the pending case with its body expression.
   * @tparam BodyExpression Type of the body expression.
   * @param body Body expression.
   * @return A @ref switch_expression with the completed case appended.
   */
  template<typename BodyExpression>
  [[nodiscard]] constexpr auto then_(BodyExpression&& body) && //
    noexcept(then_is_nothrow<ConditionExpression&&,
                             DefaultExpression&&,
                             case_types&&,
                             LabelExpression&&,
                             BodyExpression>())
  {
    using case_type = case_expression<LabelExpression, make_deferred_t<BodyExpression>>;
    using result = switch_expression<ConditionExpression, DefaultExpression, Cases..., case_type>;
    return result(
      std::forward<ConditionExpression>(m_condition),
      std::forward<DefaultExpression>(m_default),
      std::move(m_cases),
      case_type(std::forward<LabelExpression>(m_label), std::forward<BodyExpression>(body)),
      std::index_sequence_for<Cases...>{});
  }

  /// @copydoc then_
  template<typename BodyExpression>
  [[nodiscard]] constexpr auto then_(BodyExpression&& body) const& //
    noexcept(then_is_nothrow<ConditionExpression const&,
                             DefaultExpression const&,
                             case_types const&,
                             LabelExpression const&,
                             BodyExpression>())
  {
    using case_type = case_expression<LabelExpression, make_deferred_t<BodyExpression>>;
    using result = switch_expression<ConditionExpression, DefaultExpression, Cases..., case_type>;
    return result(m_condition,
                  m_default,
                  m_cases,
                  case_type(m_label, std::forward<BodyExpression>(body)),
                  std::index_sequence_for<Cases...>{});
  }
};

/**
 * @brief Deferred switch
 *
 * Cases are checked in the order they were added; the @c default case, when present,
 * is evaluated only if none matches.
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @tparam DefaultExpression Type of the @c default case, or @ref detail::no_default.
 * @tparam CaseExpression Types of the case expressions.
 */
template<Deferred ConditionExpression, typename DefaultExpression, Deferred... CaseExpression>
class switch_expression
{
  constexpr static inline bool finalized = !std::is_same_v<DefaultExpression, detail::no_default>;

public:
  using case_types = std::tuple<CaseExpression...>;
  using subexpression_types =
    std::conditional_t<finalized,
                       std::tuple<ConditionExpression, DefaultExpression, CaseExpression...>,
                       std::tuple<ConditionExpression, CaseExpression...>>;

  /**
   * @brief Result type of the underlying expressions (common type or variant).
   */
  using base_result_type = detail::switch_base_result_t<DefaultExpression, CaseExpression...>;

  /**
   * @brief Final result type of the switch expression.
   */
  using result_type = detail::unmatched_result_t<finalized, base_result_type>;

private:
  [[no_unique_address]] ConditionExpression m_condition;
  [[no_unique_address]] DefaultExpression m_default;
  [[no_unique_address]] case_types m_cases;

public:
  /**
   * @brief Constructs a switch_expression.
   * @tparam Condition Type of the condition expression.
   * @tparam Default Type of the default case.
   * @param condition Condition expression.
   * @param df Default case, or @ref detail::no_default.
   * @param cases Case expressions.
   */
  template<typename Condition, typename Default, typename Cases>
  constexpr explicit switch_expression(Condition&& condition, Default&& df, Cases&& cases) //
    noexcept(std::is_nothrow_constructible_v<ConditionExpression, Condition&&>
             && std::is_nothrow_constructible_v<DefaultExpression, Default&&>
             && std::is_nothrow_constructible_v<case_types, Cases&&>) :
    m_condition(std::forward<Condition>(condition)), m_default(std::forward<Default>(df)),
    m_cases(std::forward<Cases>(cases))
  { }

  /**
   * @brief Constructs a switch_expression by appending a case to the cases of another.
   *
   * The cases already present are carried over element by element rather than through
   * @c std::tuple_cat, so each is constructed exactly once and the exception guarantee
   * of the whole operation is computable.
   *
   * @tparam Condition Type of the condition expression.
   * @tparam Default Type of the default case.
   * @tparam Cases Type of the case tuple to extend.
   * @tparam Case Type of the case to append.
   * @tparam I Indices of the cases to carry over.
   * @param condition Condition expression.
   * @param df Default case, or @ref detail::no_default.
   * @param cases Cases accumulated so far.
   * @param appended Case to append after them.
   */
  template<typename Condition, typename Default, typename Cases, typename Case, std::size_t... I>
  constexpr explicit switch_expression(Condition&& condition,
                                       Default&& df,
                                       Cases&& cases,
                                       Case&& appended,
                                       std::index_sequence<I...>) //
    noexcept(std::is_nothrow_constructible_v<ConditionExpression, Condition&&>
             && std::is_nothrow_constructible_v<DefaultExpression, Default&&>
             && std::is_nothrow_constructible_v<case_types,
                                                decltype(std::get<I>(std::declval<Cases>()))...,
                                                Case&&>) :
    m_condition(std::forward<Condition>(condition)), m_default(std::forward<Default>(df)),
    m_cases(std::get<I>(std::forward<Cases>(cases))..., std::forward<Case>(appended))
  { }

private:
  /** @brief Whether starting a case with @p LabelEx cannot throw. */
  template<typename Condition, typename Default, typename Cs, typename LabelEx>
  static consteval bool case_is_nothrow()
  {
    using builder = switch_case_builder<ConditionExpression,
                                        DefaultExpression,
                                        make_deferred_t<LabelEx>,
                                        CaseExpression...>;
    return std::is_nothrow_constructible_v<builder, Condition, Default, Cs, LabelEx&&>;
  }

  /** @brief Whether finalizing the switch expression with @p DefaultEx cannot throw. */
  template<typename Condition, typename Cs, typename DefaultEx>
  static consteval bool default_is_nothrow()
  {
    using default_type = default_expression<make_deferred_t<DefaultEx>>;
    using result       = switch_expression<ConditionExpression, default_type, CaseExpression...>;
    return std::is_nothrow_constructible_v<default_type, DefaultEx&&>
           && std::is_nothrow_constructible_v<result, Condition, default_type&&, Cs>;
  }

public:
  /**
   * @brief Starts a case of the switch expression.
   *
   * A new case is checked after the cases already present and before the @c default
   * case, so it may be added to a switch expression that already has one.
   *
   * @tparam LabelEx Type of the label expression.
   * @param label Label expression to compare the condition against.
   * @return A @ref switch_case_builder awaiting the body expression.
   */
  template<typename LabelEx>
  [[nodiscard]] constexpr auto case_(LabelEx&& label) && //
    noexcept(case_is_nothrow<ConditionExpression&&, DefaultExpression&&, case_types&&, LabelEx>())
  {
    return switch_case_builder<ConditionExpression,
                               DefaultExpression,
                               make_deferred_t<LabelEx>,
                               CaseExpression...>(std::forward<ConditionExpression>(m_condition),
                                                  std::forward<DefaultExpression>(m_default),
                                                  std::move(m_cases),
                                                  std::forward<LabelEx>(label));
  }

  /// @copydoc case_
  template<typename LabelEx>
  [[nodiscard]] constexpr auto case_(LabelEx&& label) const& //
    noexcept(case_is_nothrow<ConditionExpression const&,
                             DefaultExpression const&,
                             case_types const&,
                             LabelEx>())
  {
    return switch_case_builder<ConditionExpression,
                               DefaultExpression,
                               make_deferred_t<LabelEx>,
                               CaseExpression...>(m_condition,
                                                  m_default,
                                                  m_cases,
                                                  std::forward<LabelEx>(label));
  }

  /**
   * @brief Finalizes the switch expression with a @c default case.
   * @tparam DefaultEx Type of the default expression.
   * @param df Default expression.
   * @return A finalized @ref switch_expression.
   */
  template<typename DefaultEx>
  [[nodiscard]] constexpr auto default_(DefaultEx&& df) && //
    noexcept(default_is_nothrow<ConditionExpression&&, case_types&&, DefaultEx>())
    requires(!finalized)
  {
    using default_type = default_expression<make_deferred_t<DefaultEx>>;
    return switch_expression<ConditionExpression, default_type, CaseExpression...>(
      std::forward<ConditionExpression>(m_condition),
      default_type(std::forward<DefaultEx>(df)),
      std::move(m_cases));
  }

  /// @copydoc default_
  template<typename DefaultEx>
  [[nodiscard]] constexpr auto default_(DefaultEx&& df) const& //
    noexcept(default_is_nothrow<ConditionExpression const&, case_types const&, DefaultEx>())
    requires(!finalized)
  {
    using default_type = default_expression<make_deferred_t<DefaultEx>>;
    return switch_expression<ConditionExpression, default_type, CaseExpression...>(
      m_condition,
      default_type(std::forward<DefaultEx>(df)),
      m_cases);
  }

private:
  /**
   * @brief Traverses the cases until one matches.
   *
   * If none does, the @c default case is evaluated, or nothing is returned when the
   * switch expression has no @c default case.
   */
  template<std::size_t I, typename Self, typename T>
  [[nodiscard]] static constexpr result_type
  choose_case(Self&& self, T const& t) noexcept(detail::is_nothrow_evaluable_v<Self>)
  {
    if constexpr (I < sizeof...(CaseExpression))
    {
      if (std::get<I>(self.m_cases).compare(t))
      {
        return detail::map_result<base_result_type>([&] { return std::get<I>(self.m_cases)(); });
      }

      return choose_case<I + 1>(std::forward<Self>(self), t);
    }
    else if constexpr (finalized)
    {
      return detail::map_result<base_result_type>([&] { return self.m_default(); });
    }
    else if constexpr (!std::is_void_v<result_type>)
    {
      return std::nullopt;
    }
  }

public:
  /**
   * @brief Evaluates the switch expression.
   * @return Result of the switch expression.
   */
  [[nodiscard]] constexpr result_type
  operator()() const noexcept(detail::is_nothrow_evaluable_v<switch_expression const&>)
  {
    return choose_case<0>(*this, evaluate(m_condition));
  }

  /// @copydoc switch_expression::operator()() const
  [[nodiscard]] constexpr result_type
  operator()() noexcept(detail::is_nothrow_evaluable_v<switch_expression&>)
  {
    return choose_case<0>(*this, evaluate(m_condition));
  }

  /**
   * @brief Visits the switch expression with a visitor.
   * @tparam Visitor Type of the visitor.
   * @param v The visitor.
   * @param nesting Nesting level.
   */
  template<typename Visitor>
  constexpr void visit(Visitor&& v, std::size_t nesting = 0) const
    noexcept(detail::is_nothrow_visitable_v<Visitor, switch_expression, subexpression_types>)
  {
    v(*this, nesting);
    m_condition.visit(v, nesting + 1);
    if constexpr (finalized)
    {
      m_default.visit(v, nesting + 1);
    }
    detail::visit_children(m_cases, v, nesting + 1);
  }
};

/**
 * @brief Starts a deferred switch expression on @p condition.
 *
 * Each case is checked in order; if none matches, the result is that of the
 * @c default case, or an empty @c std::optional when there is no @c default case.
 *
 * Example:
 * @code
 * auto var = variable<int>();
 * auto ex  = switch_(var)
 *              .case_(10).then_([] { return "10"; })
 *              .case_([] { return foo(); }).then_([] { return "result of foo"; })
 *              .default_("unknown");
 * auto expanded = ex.case_(11).then_([] { return "11"; });
 * @endcode
 *
 * @tparam ConditionExpression Type of the condition expression.
 * @param condition Condition expression.
 * @return A @ref switch_expression with no cases.
 */
template<typename ConditionExpression>
[[nodiscard]] constexpr auto switch_(ConditionExpression&& condition) noexcept(
  std::is_nothrow_constructible_v<make_deferred_t<ConditionExpression>, ConditionExpression&&>)
{
  return switch_expression<make_deferred_t<ConditionExpression>, detail::no_default>(
    std::forward<ConditionExpression>(condition),
    detail::no_default{},
    std::tuple<>{});
}

} // namespace deferred

#endif
