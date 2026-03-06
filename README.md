# deferred

Library for creating deferred evaluation expressions in C++.

``deferred`` provides:
- functions to declare constants and variables,
- functions to create deferred evaluation expressions from functions,
- ``deferred``-enabled commonly used operators.

Requirements
------------

- C++17 capable compiler.
- CMake 3.28.1 and higher.

Compilers tested:
- Clang 16.0.0 (macOS)
- GCC 13.3.0 (Ubuntu)
- MSVC 19.44 (Windows)

Building
------------

Standard CMake build process:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Installation
------------

The library is header-only. You can:
1. Copy the `include/deferred` directory to your project.
2. Install via CMake:
   ```bash
   cmake --install build --prefix /path/to/install
   ```
3. Use as a sub-project in CMake:
   ```cmake
   add_subdirectory(deferred)
   target_link_libraries(my_target PRIVATE deferred::deferred)
   ```

Documentation
------------

Generate documentation with Doxygen:

```bash
# From the build directory
cmake --build . --target documentation
```
The output will be in the ``build/html`` directory.

Usage
------------

```C++
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
```

Examples can be found in the ``examples/`` directory. They are compiled by default.

Testing
------------

Tests are written using [Catch2 v3](https://github.com/catchorg/Catch2).
To run tests after building:

```bash
ctest --output-on-failure
```

Known Issues
------------

* Using callable objects and functions with side-effects in ``invoke()`` and ``if_then_else()`` will result in a constant expression, assignable to ``constant_``.
* Using ``deferred`` with other expression template based classes (e.g., `std::valarray`) may not be possible.
