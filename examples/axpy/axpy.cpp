/*
 * Copyright (c) 2019 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <iostream>

#include "deferred/deferred.hpp"
#include "deferred/type_name.hpp"

int main()
{
  using namespace deferred;

  auto a = variable<double>();
  auto x = variable<double>();
  auto y = variable<double>();

  auto z = a * x + y;

  a = 2.0;
  x = 3.0;
  y = 1.0;

  std::cout << "Expression: " << type_name(z) << '\n';
  std::cout << z() << '\n';

  return 0;
}
