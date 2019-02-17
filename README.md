# deferred

Library for creating deferred evaluation expressions in C++.

``deferred`` provides:
- functions to declare constants and variables,
- functions to create deferred evaluation expressions from functions,
- ``deferred``-enabled commonly used operators.

Requirements
------------

- C++17 capable compiler.
- CMake 3.8 and higher.

Compilers tested:
- Clang 8.0.0
- gcc 8.2.0

Installation
------------

Copy ``include/deferred`` and include it in your compiler line with the ``-I`` flag.

Documentation
------------

Generate documentation with ``make documentation`` from your build directory.

Usage
------------

.. code:: C++

    // examples/trivial/main.cpp
    #include <iostream>
    #include <deferred/deferred.hpp>
    
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

Examples can be found in the ``examples/`` directory. They are compiled by default.

Testing
------------

Tests are written using [Catch2](https://github.com/catchorg/Catch2).

Known Issues
------------

* Using callable objects and functions with side-effects in ``invoke()`` and ``if_then_else()`` will result in a constant expression, assignable to ``constant_``.
* Using ``deferred`` with other expression template based classes (e.g., `std::valarray`) may not be possible.
