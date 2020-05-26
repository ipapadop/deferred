/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <deferred/deferred.hpp>
#include <iostream>

int main()
{
  auto v = deferred::variable<int>();
  auto x = deferred::constant(2);
  auto y = deferred::constant(3);

  auto expression = (v * x) + y;

  v = 10;

  auto res = expression();

  std::cout << res << "==" << (10 * 2) + 3 << '\n';

  return 0;
}
