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

template<size_t Index = 0, // start iteration at 0 index
         typename TTuple,  // the tuple type
         size_t Size = std::tuple_size_v<std::remove_reference_t<TTuple>>, // tuple
                                                                           // size
         typename TCallable, // the callable to bo invoked for each tuple item
         typename... TArgs   // other arguments to be passed to the callable
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

  auto v = deferred::variable<int>();
  auto x = deferred::constant(2);
  auto y = deferred::constant(3);

  auto expression = (v * x) + y;

  v = 10;

  auto res = expression();

  std::cout << res << "==" << (10 * 2) + 3 << '\n';

  return 0;
#if 0
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
  //  std::cout << deferred::type_name<decltype(da)>().c_str() << '\n';
  //  std::cout << deferred::type_name<decltype(da())>().c_str() << '\n';
  auto dx = constant(x);
  //  std::cout << deferred::type_name<decltype(dx)>().c_str() << '\n';
  //  std::cout << deferred::type_name<decltype(dx())>().c_str() << '\n';
  auto dy = constant(y);
  //  std::cout << deferred::type_name<decltype(dy)>().c_str() << '\n';
  //  std::cout << deferred::type_name<decltype(dy())>().c_str() << '\n';

#if 1
  auto partial_dres = da * dx;
  // std::cout << deferred::type_name<decltype(partial_dres)>().c_str() << '\n';
  std::valarray<float> partial_eval_res = partial_dres();
  std::cout << "partial deferred result: ";
  std::copy(std::cbegin(partial_eval_res),
            std::cend(partial_eval_res),
            std::ostream_iterator<float>(std::cout, " "));
  std::cout << "\n\n";
#endif

  // partial_dres.visit(print_visitor{});

  auto dres = partial_dres + dy;
  // std::cout << deferred::type_name<decltype(dres)>().c_str() << '\n';
  // std::cout << deferred::type_name<decltype(dres())>().c_str() << '\n';

  dres.visit(print_visitor{});

  std::valarray<float> eval_res = dres();
  std::cout << "deferred result: ";
  std::copy(std::cbegin(eval_res),
            std::cend(eval_res),
            std::ostream_iterator<float>(std::cout, " "));
  std::cout << '\n';
#endif
  return 0;
}
