/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <iterator>
#include <iostream>
#include <vector>

#include "deferred/deferred.hpp"

// Adds vectors x and y
template<typename T>
std::vector<T> add_vectors(std::vector<T> x, std::vector<T> const& y)
{
  std::transform(x.begin(), x.end(), y.begin(), x.begin(), std::plus<>{});
  return x;
}

// Multiplies every element of x with v
template<typename T>
std::vector<T> multiply_scalar_vector(T const& v, std::vector<T> x)
{
  std::for_each(x.begin(), x.end(), [&](auto& t) { t *= v; });
  return x;
}

int main()
{
  using namespace deferred;

  auto a = 2.0f;
  std::vector<float> x(10, 0.1f);
  std::vector<float> y(10, 0.2f);
  auto mult = multiply_scalar_vector(a, x);
  auto res = add_vectors(std::move(mult), y);

  std::cout << "vector result:   ";
  std::copy(std::cbegin(res), std::cend(res), std::ostream_iterator<float>(std::cout, " "));
  std::cout << '\n';

  auto da    = constant(a);
  auto dx    = constant(x);
  auto dy    = constant(y);
  auto dmult = invoke(multiply_scalar_vector<float>, da, dx);
  auto dres  = invoke(add_vectors<float>, dmult, dy);

  auto eval_res = dres();
  std::cout << "deferred result: ";
  std::copy(std::cbegin(eval_res), std::cend(eval_res), std::ostream_iterator<float>(std::cout, " "));
  std::cout << '\n';

  return 0;
}
