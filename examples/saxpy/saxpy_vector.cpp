// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

#include "deferred/deferred.hpp"

/**
 * @brief Adds elements of vector @p x and vector @p y pairwise.
 * @tparam T The element type.
 * @param x The first vector.
 * @param y The second vector.
 * @return A new vector containing the sum of @p x and @p y.
 */
template<typename T>
std::vector<T> add_vectors(std::vector<T> x, std::vector<T> const& y)
{
  std::transform(x.begin(), x.end(), y.begin(), x.begin(), std::plus<>{});
  return x;
}

/**
 * @brief Multiplies every element of vector @p x with a scalar @p v.
 * @tparam T The element type.
 * @param v The scalar value.
 * @param x The vector.
 * @return A new vector with each element multiplied by @p v.
 */
template<typename T>
std::vector<T> multiply_scalar_vector(T const& v, std::vector<T> x)
{
  std::for_each(x.begin(), x.end(), [&](auto& t) { t *= v; });
  return x;
}

/// @brief Main function demonstrating deferred evaluation with std::vector.
int main()
{
  using namespace deferred;

  auto a = 2.0f;
  std::vector<float> x(10, 0.1f);
  std::vector<float> y(10, 0.2f);
  auto mult = multiply_scalar_vector(a, x);
  auto res  = add_vectors(std::move(mult), y);

  std::cout << "vector result:   ";
  std::copy(std::begin(res), std::end(res), std::ostream_iterator<float>(std::cout, " "));
  std::cout << '\n';

  auto da    = constant(a);
  auto dx    = constant(x);
  auto dy    = constant(y);
  auto dmult = invoke(multiply_scalar_vector<float>, da, dx);
  auto dres  = invoke(add_vectors<float>, dmult, dy);

  auto eval_res = dres();
  std::cout << "deferred result: ";
  std::copy(std::begin(eval_res), std::end(eval_res), std::ostream_iterator<float>(std::cout, " "));
  std::cout << '\n';

  return 0;
}
