// SPDX-FileCopyrightText: 2019-2026 Yiannis Papadopoulos <giannis.papadopoulos@gmail.com>
// SPDX-License-Identifier: MIT

#include <deferred/deferred.hpp>
#include <iostream>

/**
 * @brief Trivial example showing basic variable setup and deferred evaluation.
 */
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
