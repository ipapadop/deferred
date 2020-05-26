/*
 * Copyright (c) 2019-2020 Yiannis Papadopoulos
 *
 * Distributed under the terms of the MIT License.
 *
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 */

#include <iostream>
#include <iterator>
#include <valarray>

#include "deferred/deferred.hpp"
#include "deferred/type_name.hpp"

struct print_visitor
{
  std::string indentation(std::size_t indent) const
  {
    return std::string(indent, '\t');
  }

  template<typename T>
  void operator()(T&& t, std::size_t nesting) const
  {
    auto i = indentation(nesting);

    std::cout
      << i << "Expression: " << deferred::type_name<decltype(t)>() << '\n'
      << i << "Result type: " << deferred::type_name<decltype(t())>() << '\n';
  }
};


int main()
{
  using namespace deferred;

  auto a = 2.0f;
  std::valarray<float> x(0.1f, 10);
  std::valarray<float> y(0.2f, 10);
  std::valarray<float> res = a * x + y;

  std::cout << "valarray result: ";
  std::copy(std::begin(res),
            std::end(res),
            std::ostream_iterator<float>(std::cout, " "));
  std::cout << "\n\n";

  auto da = constant(a);
  auto dx = constant(x);
  auto dy = constant(y);

  // this is fine
  auto partial_dres = da * dx;

  std::valarray<float> partial_eval_res = partial_dres();
  std::cout << "partial deferred result: ";
  std::copy(std::begin(partial_eval_res),
            std::end(partial_eval_res),
            std::ostream_iterator<float>(std::cout, " "));
  std::cout << "\n\n";

  // FIXME this is not
  auto dres = partial_dres + dy;

  dres.visit(print_visitor{});

  std::valarray<float> eval_res = dres();
  std::cout << "deferred result: ";
  std::copy(std::begin(eval_res),
            std::end(eval_res),
            std::ostream_iterator<float>(std::cout, " "));
  std::cout << '\n';

  return 0;
}
