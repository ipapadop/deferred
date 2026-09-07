# deferred

Library for creating deferred evaluation expressions in C++23.

``deferred`` provides:
- functions to declare constants and variables,
- functions to create deferred evaluation expressions from functions,
- expandable deferred switch expressions,
- ``deferred``-enabled commonly used operators.

Requirements
------------

- C++23 capable compiler.
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

Visiting expressions
--------------------

Deferred expressions provide a ``visit`` member for inspecting their expression
tree without evaluating it. Traversal is preorder and passes each structural
node with its nesting level to the visitor:

```C++
expression.visit([](auto const& node, std::size_t nesting) {
  // Inspect node at nesting depth.
});
```

Composite nodes visit their children from left to right. Constants and variables
are leaves; their stored values and expression operator objects are not visited.
Conditional branch, switch case, and switch default wrappers are structural
nodes and are included in the traversal. Nodes are exposed as read-only
references. A visitor may be passed as an lvalue or rvalue, but traversal invokes
the same visitor object as an lvalue for every node.

Invoking member pointers
------------------------

``deferred::invoke`` follows the standard ``std::invoke`` model, including
pointers to member functions and member data:

```C++
struct counter {
  int value{};
  int read() const noexcept { return value; }
  void increment() noexcept { ++value; }
};

counter c;
auto read_copy = deferred::invoke(&counter::read, c);
auto increment_original = deferred::invoke(&counter::increment, std::ref(c));
auto value_reference = deferred::invoke(&counter::value, &c);
```

Ordinary arguments retain the library's existing ownership behavior and are
stored as constants, so ``read_copy`` operates on a copy. Use a pointer,
``std::ref``, or ``std::cref`` when the deferred expression should refer to the
original object. Owned constants expose their values as const references, so
non-const member functions require a pointer or ``std::ref``.

Applying a tuple of arguments
-----------------------------

``deferred::apply`` is the tuple form of ``deferred::invoke``, standing in the
same relation to it as ``std::apply`` does to ``std::invoke``. It *builds* a
deferred expression rather than evaluating one:

```C++
auto from_pack  = deferred::invoke(std::plus<>{}, 1, 2);
auto from_tuple = deferred::apply(std::plus<>{}, std::make_tuple(1, 2));

// identical types; neither has run yet
static_assert(std::is_same_v<decltype(from_pack), decltype(from_tuple)>);
int result = from_tuple();  // 3
```

Everything ``invoke`` supports carries over, including member pointers and the
storage rules described above.

Exception guarantees
--------------------

Deferred storage, invocation, recursive evaluation, control-flow expressions,
and visitor traversal propagate ``noexcept`` from the user operations and value
construction they perform. Operations involving a potentially throwing callable,
conversion, move, comparison, or visitor remain potentially throwing.

Testing
------------

Tests are written using [Catch2 v3](https://github.com/catchorg/Catch2).
To run tests after building:

```bash
ctest --output-on-failure
```

CI also runs the suite under AddressSanitizer. To reproduce that build locally:

```bash
cmake -S . -B build-asan -D CMAKE_BUILD_TYPE=Debug \
  -D CMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g"
cmake --build build-asan --parallel
ASAN_OPTIONS=detect_stack_use_after_return=1 ctest --test-dir build-asan --output-on-failure
```

Formatting
----------

The project uses `clang-format` for code formatting.

To format all files:
```bash
# From the build directory
cmake --build . --target format
```

To format only changed files (requires `git`):
```bash
./scripts/format_changed.sh
```

Doxygen comments must use the block style `/** ... */` for multi-line documentation and `///` for single-line comments. Always start with a `@brief` description. Use `@` instead of `\` for Doxygen commands (e.g., `@param`, `@tparam`, `@return`).
