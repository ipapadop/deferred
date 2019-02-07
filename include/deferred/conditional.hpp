/** @file */
/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#ifndef DEFERRED_CONDITIONAL_HPP
#define DEFERRED_CONDITIONAL_HPP

#include <utility>

#include "make_deferred.hpp"

namespace deferred
{

/**
 * TODO
 */
template<typename Expression>
class if_expression_
  : private Expression
{
public:
  template<typename Ex>
  constexpr explicit if_expression_(Ex&& ex)
    : Expression(std::forward<Ex>(ex))
  {}

  constexpr void operator()() const
  {
    Expression::operator()();
  }

  template<typename Visitor>
  constexpr decltype(auto) visit(Visitor&& v) const
  {
    return std::forward<Visitor>(v)(*this);
  }
};

/**
 * TODO
 */
template<typename Expression>
constexpr auto if_(Expression&& ex)
{
  using expression_type = make_deferred_t<std::decay_t<Expression>>;
  return if_expression_<expression_type>(std::forward<Expression>(ex));
}

} // namespace deferred

#endif
