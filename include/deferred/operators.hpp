// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#ifndef DEFERRED_OPERATORS_HPP
#define DEFERRED_OPERATORS_HPP

#include <functional>
#include <utility>

#include "assign.hpp"
#include "invoke.hpp"
#include "type_traits/is_deferred.hpp"

namespace deferred {

namespace detail {

/** @brief Implements deferred unary plus. */
struct unary_plus
{
  /// @brief Applies unary plus to @p value.
  template<typename T>
  constexpr auto operator()(T&& value) const noexcept(noexcept(+std::forward<T>(value)))
  {
    return +std::forward<T>(value);
  }
};

/** @brief Implements deferred prefix increment. */
struct pre_increment
{
  /// @brief Prefix-increments @p value.
  template<typename T>
  constexpr auto operator()(T&& value) const noexcept(noexcept(++std::forward<T>(value)))
  {
    return ++std::forward<T>(value);
  }
};

/** @brief Implements deferred postfix increment. */
struct post_increment
{
  /// @brief Postfix-increments @p value.
  template<typename T>
  constexpr auto operator()(T&& value) const noexcept(noexcept(std::forward<T>(value)++))
  {
    return std::forward<T>(value)++;
  }
};

/** @brief Implements deferred prefix decrement. */
struct pre_decrement
{
  /// @brief Prefix-decrements @p value.
  template<typename T>
  constexpr auto operator()(T&& value) const noexcept(noexcept(--std::forward<T>(value)))
  {
    return --std::forward<T>(value);
  }
};

/** @brief Implements deferred postfix decrement. */
struct post_decrement
{
  /// @brief Postfix-decrements @p value.
  template<typename T>
  constexpr auto operator()(T&& value) const noexcept(noexcept(std::forward<T>(value)--))
  {
    return std::forward<T>(value)--;
  }
};

/** @brief Implements deferred left shift. */
struct shift_left
{
  /// @brief Shifts @p left left by @p right.
  template<typename T, typename U>
  constexpr auto operator()(T&& left, U&& right) const
    noexcept(noexcept(std::forward<T>(left) << std::forward<U>(right)))
  {
    return std::forward<T>(left) << std::forward<U>(right);
  }
};

/** @brief Implements deferred right shift. */
struct shift_right
{
  /// @brief Shifts @p left right by @p right.
  template<typename T, typename U>
  constexpr auto operator()(T&& left, U&& right) const
    noexcept(noexcept(std::forward<T>(left) >> std::forward<U>(right)))
  {
    return std::forward<T>(left) >> std::forward<U>(right);
  }
};

/** @brief Implements deferred += assignment. */
struct plus_assign
{
  /// @brief Applies <tt>+=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) += std::forward<U>(value)))
  {
    return std::forward<T>(target) += std::forward<U>(value);
  }
};

/** @brief Implements deferred -= assignment. */
struct minus_assign
{
  /// @brief Applies <tt>-=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) -= std::forward<U>(value)))
  {
    return std::forward<T>(target) -= std::forward<U>(value);
  }
};

/** @brief Implements deferred *= assignment. */
struct multiplies_assign
{
  /// @brief Applies <tt>*=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) *= std::forward<U>(value)))
  {
    return std::forward<T>(target) *= std::forward<U>(value);
  }
};

/** @brief Implements deferred /= assignment. */
struct divides_assign
{
  /// @brief Applies <tt>/=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) /= std::forward<U>(value)))
  {
    return std::forward<T>(target) /= std::forward<U>(value);
  }
};

/** @brief Implements deferred %= assignment. */
struct modulus_assign
{
  /// @brief Applies <tt>%=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) %= std::forward<U>(value)))
  {
    return std::forward<T>(target) %= std::forward<U>(value);
  }
};

/** @brief Implements deferred &= assignment. */
struct bit_and_assign
{
  /// @brief Applies <tt>&=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) &= std::forward<U>(value)))
  {
    return std::forward<T>(target) &= std::forward<U>(value);
  }
};

/** @brief Implements deferred |= assignment. */
struct bit_or_assign
{
  /// @brief Applies <tt>|=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) |= std::forward<U>(value)))
  {
    return std::forward<T>(target) |= std::forward<U>(value);
  }
};

/** @brief Implements deferred ^= assignment. */
struct bit_xor_assign
{
  /// @brief Applies <tt>^=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) ^= std::forward<U>(value)))
  {
    return std::forward<T>(target) ^= std::forward<U>(value);
  }
};

/** @brief Implements deferred <<= assignment. */
struct shift_left_assign
{
  /// @brief Applies <tt><<=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) <<= std::forward<U>(value)))
  {
    return std::forward<T>(target) <<= std::forward<U>(value);
  }
};

/** @brief Implements deferred >>= assignment. */
struct shift_right_assign
{
  /// @brief Applies <tt>>>=</tt> to @p target with @p value.
  template<typename T, typename U>
  constexpr decltype(auto) operator()(T&& target, U&& value) const
    noexcept(noexcept(std::forward<T>(target) >>= std::forward<U>(value)))
  {
    return std::forward<T>(target) >>= std::forward<U>(value);
  }
};

} // namespace detail

/**
 * @brief Deferred binary operator +
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator+(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::plus<>, T, U>())
{
  return invoke(std::plus<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator -
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator-(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::minus<>, T, U>())
{
  return invoke(std::minus<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred prefix or unary operator +
 * @tparam T Type of the operand.
 * @param t Operand.
 * @return A deferred expression representing the operation.
 */
template<Deferred T>
[[nodiscard]] constexpr auto
operator+(T&& t) noexcept(detail::invoke_is_nothrow<detail::unary_plus, T>())
{
  return invoke(detail::unary_plus{}, std::forward<T>(t));
}

/**
 * @brief Deferred prefix or unary operator -
 * @tparam T Type of the operand.
 * @param t Operand.
 * @return A deferred expression representing the operation.
 */
template<Deferred T>
[[nodiscard]] constexpr auto
operator-(T&& t) noexcept(detail::invoke_is_nothrow<std::negate<>, T>())
{
  return invoke(std::negate<>{}, std::forward<T>(t));
}

/**
 * @brief Deferred binary operator *
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator*(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::multiplies<>, T, U>())
{
  return invoke(std::multiplies<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator /
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator/(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::divides<>, T, U>())
{
  return invoke(std::divides<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator %
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator%(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::modulus<>, T, U>())
{
  return invoke(std::modulus<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred prefix or unary operator ++
 * @tparam T Type of the operand.
 * @param t Operand.
 * @return A deferred expression representing the operation.
 */
template<Deferred T>
[[nodiscard]] constexpr auto
operator++(T&& t) noexcept(detail::invoke_is_nothrow<detail::pre_increment, T>())
{
  return invoke(detail::pre_increment{}, std::forward<T>(t));
}

/**
 * @brief Deferred postfix operator ++
 * @tparam T Type of the operand.
 * @param t Operand.
 * @param \<unnamed\> An int for postfix differentiation.
 * @return A deferred expression representing the operation.
 */
template<Deferred T>
[[nodiscard]] constexpr auto
operator++(T&& t, int) noexcept(detail::invoke_is_nothrow<detail::post_increment, T>())
{
  return invoke(detail::post_increment{}, std::forward<T>(t));
}

/**
 * @brief Deferred prefix or unary operator --
 * @tparam T Type of the operand.
 * @param t Operand.
 * @return A deferred expression representing the operation.
 */
template<Deferred T>
[[nodiscard]] constexpr auto
operator--(T&& t) noexcept(detail::invoke_is_nothrow<detail::pre_decrement, T>())
{
  return invoke(detail::pre_decrement{}, std::forward<T>(t));
}

/**
 * @brief Deferred postfix operator --
 * @tparam T Type of the operand.
 * @param t Operand.
 * @param \<unnamed\> An int for postfix differentiation.
 * @return A deferred expression representing the operation.
 */
template<Deferred T>
[[nodiscard]] constexpr auto
operator--(T&& t, int) noexcept(detail::invoke_is_nothrow<detail::post_decrement, T>())
{
  return invoke(detail::post_decrement{}, std::forward<T>(t));
}

/**
 * @brief Deferred binary operator ==
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator==(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::equal_to<>, T, U>())
{
  return invoke(std::equal_to<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator !=
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator!=(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::not_equal_to<>, T, U>())
{
  return invoke(std::not_equal_to<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator >
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator>(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::greater<>, T, U>())
{
  return invoke(std::greater<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator <
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator<(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::less<>, T, U>())
{
  return invoke(std::less<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator >=
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator>=(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::greater_equal<>, T, U>())
{
  return invoke(std::greater_equal<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator <=
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator<=(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::less_equal<>, T, U>())
{
  return invoke(std::less_equal<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator &&
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator&&(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::logical_and<>, T, U>())
{
  return invoke(std::logical_and<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator ||
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator||(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::logical_or<>, T, U>())
{
  return invoke(std::logical_or<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred prefix or unary operator !
 * @tparam T Type of the operand.
 * @param t Operand.
 * @return A deferred expression representing the operation.
 */
template<Deferred T>
[[nodiscard]] constexpr auto
operator!(T&& t) noexcept(detail::invoke_is_nothrow<std::logical_not<>, T>())
{
  return invoke(std::logical_not<>{}, std::forward<T>(t));
}

/**
 * @brief Deferred binary operator &
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator&(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::bit_and<>, T, U>())
{
  return invoke(std::bit_and<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator |
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator|(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::bit_or<>, T, U>())
{
  return invoke(std::bit_or<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator ^
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator^(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<std::bit_xor<>, T, U>())
{
  return invoke(std::bit_xor<>{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred prefix or unary operator ~
 * @tparam T Type of the operand.
 * @param t Operand.
 * @return A deferred expression representing the operation.
 */
template<Deferred T>
[[nodiscard]] constexpr auto
operator~(T&& t) noexcept(detail::invoke_is_nothrow<std::bit_not<>, T>())
{
  return invoke(std::bit_not<>{}, std::forward<T>(t));
}

/**
 * @brief Deferred binary operator <<
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator<<(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<detail::shift_left, T, U>())
{
  return invoke(detail::shift_left{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred binary operator >>
 * @tparam T Type of the left operand.
 * @tparam U Type of the right operand.
 * @param t Left operand.
 * @param u Right operand.
 * @return A deferred expression representing the operation.
 */
template<typename T, typename U>
  requires AnyDeferred<T, U>
[[nodiscard]] constexpr auto
operator>>(T&& t, U&& u) noexcept(detail::invoke_is_nothrow<detail::shift_right, T, U>())
{
  return invoke(detail::shift_right{}, std::forward<T>(t), std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator +=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt>+=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator+=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::plus_assign, T, U>())
{
  return detail::assign_expression_t<detail::plus_assign, T, U>(std::forward<T>(t),
                                                                std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator -=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt>-=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator-=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::minus_assign, T, U>())
{
  return detail::assign_expression_t<detail::minus_assign, T, U>(std::forward<T>(t),
                                                                 std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator *=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt>*=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator*=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::multiplies_assign, T, U>())
{
  return detail::assign_expression_t<detail::multiplies_assign, T, U>(std::forward<T>(t),
                                                                      std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator /=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt>/=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator/=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::divides_assign, T, U>())
{
  return detail::assign_expression_t<detail::divides_assign, T, U>(std::forward<T>(t),
                                                                   std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator %=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt>%=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator%=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::modulus_assign, T, U>())
{
  return detail::assign_expression_t<detail::modulus_assign, T, U>(std::forward<T>(t),
                                                                   std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator &=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt>&=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator&=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::bit_and_assign, T, U>())
{
  return detail::assign_expression_t<detail::bit_and_assign, T, U>(std::forward<T>(t),
                                                                   std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator |=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt>|=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator|=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::bit_or_assign, T, U>())
{
  return detail::assign_expression_t<detail::bit_or_assign, T, U>(std::forward<T>(t),
                                                                  std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator ^=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt>^=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator^=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::bit_xor_assign, T, U>())
{
  return detail::assign_expression_t<detail::bit_xor_assign, T, U>(std::forward<T>(t),
                                                                   std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator <<=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt><<=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator<<=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::shift_left_assign, T, U>())
{
  return detail::assign_expression_t<detail::shift_left_assign, T, U>(std::forward<T>(t),
                                                                      std::forward<U>(u));
}

/**
 * @brief Deferred compound assignment operator >>=
 * @tparam T Type of the assignment target.
 * @tparam U Type of the right operand.
 * @param t Assignment target; must be a deferred expression yielding an lvalue.
 * @param u Right operand.
 * @return An @ref assign_expression performing <tt>>>=</tt> when evaluated.
 */
template<typename T, typename U>
  requires Deferred<T>
[[nodiscard]] constexpr auto
operator>>=(T&& t, U&& u) noexcept(detail::assign_is_nothrow<detail::shift_right_assign, T, U>())
{
  return detail::assign_expression_t<detail::shift_right_assign, T, U>(std::forward<T>(t),
                                                                       std::forward<U>(u));
}

} // namespace deferred

#endif
