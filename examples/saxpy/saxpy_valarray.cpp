/*
 * Copyright (c) 2019 Yiannis Papadopoulos
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

#include <functional> // std::invoke
#include <tuple>      // std::tuple

// from
// https://stackoverflow.com/questions/1198260/how-can-you-iterate-over-the-elements-of-an-stdtuple
template<
  size_t Index = 0,
  typename TTuple,
  size_t Size = std::tuple_size_v<std::remove_reference_t<TTuple>>,
  typename TCallable,
  typename... TArgs
  >
void for_each(TTuple&& tuple, TCallable&& callable, TArgs&&... args)
{
  if constexpr (Index < Size)
  {
    std::invoke(callable, args..., std::get<Index>(tuple));

    if constexpr (Index + 1 < Size)
      for_each<Index + 1>(std::forward<TTuple>(tuple),
                          std::forward<TCallable>(callable),
                          std::forward<TArgs>(args)...);
  }
}

struct print_visitor
{
  std::size_t indent = 0;

  std::string indentation() const
  {
    return std::string(indent, '\t');
  }

  template<typename T, std::enable_if_t<deferred::is_constant_v<T>>* = nullptr>
  void operator()(T&& t) const
  {
    std::cout << indentation() << "constant ("
              << deferred::type_name<decltype(t())>() << ")\n";
  }

  template<typename T, std::enable_if_t<deferred::is_variable_v<T>>* = nullptr>
  void operator()(T&& t) const
  {
    std::cout << indentation() << "variable ("
              << deferred::type_name<decltype(t())>() << ")\n";
  }

  template<typename T, std::enable_if_t<deferred::is_expression_v<T>>* = nullptr>
  void operator()(T&& t)
  {
    using operator_type = typename std::decay_t<T>::operator_type;

    std::cout << indentation() << "expression:\n";

    ++indent;

    auto i = indentation();

    std::cout << i << "result type (" << deferred::type_name<decltype(t())>()
              << ')' << '\n'
              << i << "operator ("
              << deferred::type_name<std::decay_t<operator_type>>() << ')'
              << '\n';

    for_each(t.subexpressions(), *this);

    --indent;
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
  std::copy(std::cbegin(res),
            std::cend(res),
            std::ostream_iterator<float>(std::cout, " "));
  std::cout << "\n\n";

  auto da = constant(a);
  auto dx = constant(x);
  auto dy = constant(y);

  // this is fine
  auto partial_dres = da * dx;

  std::valarray<float> partial_eval_res = partial_dres();
  std::cout << "partial deferred result: ";
  std::copy(std::cbegin(partial_eval_res),
            std::cend(partial_eval_res),
            std::ostream_iterator<float>(std::cout, " "));
  std::cout << "\n\n";

  // FIXME this is not
  auto dres = partial_dres + dy;

  dres.visit(print_visitor{});

  std::valarray<float> eval_res = dres();
  std::cout << "deferred result: ";
  std::copy(std::cbegin(eval_res),
            std::cend(eval_res),
            std::ostream_iterator<float>(std::cout, " "));
  std::cout << '\n';

  return 0;
}
