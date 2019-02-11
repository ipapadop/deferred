/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
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

#include "type_traits/is_expression.hpp"

namespace deferred
{

/**
 * TODO
 */
template<typename Operator, typename... Expressions>
class expression_
  : private Operator,
    private std::tuple<Expressions...>
{
public:
  template<typename Op, typename... Ex>
  constexpr explicit expression_(Op&& op, Ex&&... ex)
    : Operator(std::forward<Op>(op)),
      std::tuple<Expressions...>(std::forward<Ex>(ex)...)
  {}

private:
  template<std::size_t... I>
  constexpr decltype(auto) call(std::index_sequence<I...>) const
  {
    return Operator::operator()(std::get<I>(*this)()...);
  }

public:
  constexpr decltype(auto) operator()() const
  {
    return call(std::index_sequence_for<Expressions...>{});
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

} // namespace deferred

#endif
