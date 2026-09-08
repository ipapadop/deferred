# Agent Context: Deferred Evaluation Library

This project is a C++23 header-only library for creating deferred evaluation expressions.

## Project Overview

- **Purpose**: Provides a mechanism to define expressions (using constants, variables, and operators) that are evaluated lazily at a later point.
- **Control flow**: `if_`, `switch_`, `while_`, `do_` and `for_` are built by chaining:
  `if_(c).then_(a).else_if_(c2).then_(b).else_(d)`, `switch_(c).case_(l).then_(b).default_(d)`, `while_(c).do_(b)`,
  `do_(b).while_(c)`, `for_(init, c, step).do_(b)`. An `if_` without `else_` and a `switch_` without
  `default_` evaluate to a `std::optional`, or to `void` when every branch produces no value.
- **Loops**: loop constructs evaluate to `void` and may be evaluated more than once; `for_` re-runs its `init`
  clause on every evaluation. There is no deferred assignment operator, so an `init` or `step` clause that assigns
  must be written as a lambda (`[&i] { i = 0; }`), while `i < 10` and `++i` are deferred expressions already.
- **Visitor order**: children are visited in `subexpression_types` order, which is the source order of the
  construct, not its evaluation order — for `for_(init, c, step).do_(b)` that is init, condition, step, body.
- **Switch expressions**: Existing switch expressions can be expanded with `case_(label).then_(body)`, which adds the
  case after the existing ones and before the `default_` case.
- **Result types**: Control-flow result types are deduced with `detail::evaluated_result_t` — the type `evaluate()`
  yields — so a branch, case or default body that returns a deferred expression contributes its evaluated value type,
  and a result is never a reference into a subexpression.
- **Invocation**: Decayed callables are stored directly and evaluated with `std::invoke`, including member pointers.
- **Exception guarantees**: Template APIs use conditional `noexcept` based on stored values and user operations.
- **Main Technologies**: 
  - **Language**: C++23
  - **Build System**: CMake (3.28.1+)
  - **Testing**: Catch2 (v3.13.0)
  - **CI**: GitHub Actions (Ubuntu, Windows, macOS)
  - **Documentation**: Doxygen
- **Architecture**:
  - `include/deferred/`: Contains the library headers. `deferred.hpp` is the main entry point.
    - `include/deferred/detail/`: Private implementation traits and helpers, in `namespace deferred::detail`.
    - `include/deferred/type_traits/`: Public type traits and concepts.
  - `examples/`: Usage examples (e.g., `saxpy`, `trivial`).
  - `test/`: Unit and integration tests.
  - **Dependencies**: Catch2 is fetched automatically via CMake `FetchContent` in `test/CMakeLists.txt`.

## Building and Running

### Prerequisites
- C++23 capable compiler (Clang 16.0.0+, GCC 13.3.0+, MSVC 19.44+).
- CMake 3.28.1 or higher.

### Build Instructions
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Running Tests
After building, you can run tests using `ctest` or by executing the test binaries directly:
```bash
# Using ctest
ctest --output-on-failure

# Or directly
./test/unit/unit_tests
./test/integration/integration_tests
```

### Formatting
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

### Building Documentation
If Doxygen is found, you can generate the HTML documentation:
```bash
# From the build directory
cmake --build . --target documentation
```
The output will be in the `build/html` directory. Note that documentation is not built by default and must be explicitly built if you wish to install it.

### Installation
The library can be installed using standard CMake installation:
```bash
cmake --install . --prefix /path/to/install
```
Or simply copy the `include/deferred` directory to your project's include path.

## Development Conventions

- **Coding Style**: Adheres to the `.clang-format` file in the root directory (based on the WebKit style).
  - **Indentation**: 2 spaces.
  - **Pointer Alignment**: Left (`Type* ptr`).
  - **Brace Wrapping**: Custom (check `.clang-format` for details).
  - **Formatting**: All code must be formatted using `clang-format` before being committed. Use the `format` CMake target or the `scripts/format_changed.sh` script.
- **Naming**: 
  - Namespaces: `deferred`
  - Files: `.hpp` for headers, `.cpp` for tests/examples.
- **Documentation Comments**:
  - Use Doxygen block comments `/** ... */` for multi-line documentation. Use `///` for single-line comments.
  - Always include a `@brief` description as the first line of the comment.
  - Use `@tparam`, `@param`, `@return`, and other Doxygen commands where applicable instead of their `\ ` equivalents.
- **Testing**: New features must include unit tests in `test/unit/` and integration tests in `test/integration/`. Tests use the Catch2 v3 framework.
  - **Catch2 v3**: Uses `#include <catch2/catch_test_macros.hpp>` and requires a custom `main` in `main.cpp` using `Catch::Session().run()`.
  - **Verification**: All changes must be verified by ensuring that all tests pass (`ctest`).
- **Source Control**: NEVER commit directly to the `main` branch. Always use feature branches and pull requests for any changes.
- **Project Documentation**: After EVERY change, `README.md` and `AGENTS.md` MUST be reviewed and updated to reflect the current state of the project.
- **License**: MIT License. All source files should include the standard MIT license header.
- **Header Guards**: Use `#ifndef DEFERRED_FILENAME_HPP` format, qualified by directory for
  subdirectories (`DEFERRED_DETAIL_MAP_VOID_HPP`, `DEFERRED_TYPE_TRAITS_IS_DEFERRED_HPP`).
- **`detail/` Layering**: Headers in `include/deferred/detail/` MUST NOT include anything from the
  source root or any other directory in the library — no `#include "../foo.hpp"`. Dependencies point
  inward only: root headers may include `detail/`, never the reverse. A `detail/` header may only
  include standard headers and other `detail/` headers.
  - If an implementation helper needs something from a root header (for example `evaluate()`), it
    does not belong in `detail/` — put it in that root header, inside `namespace detail`.
- **`detail/` Header Contents**: Each header in `detail/` defines exactly one entity and is named
  after it. A trait's primary template, its specializations, and its `_v`/`_t` alias count as one
  entity and stay together, as does any helper that exists solely to implement that entity — a
  function object handed to an algorithm, a `deduce_*` metafunction. Give a helper its own header
  only when it carries meaning independently and could serve another caller.
- **`detail/` Tests**: Every header in `detail/` has a matching test file at
  `test/unit/detail/<name>.cpp`, giving a 1:1 pairing with no orphans in either direction.
- **CI**: GitHub Actions workflow (`.github/workflows/c-cpp.yml`) builds the project on Ubuntu, Windows (MSVC), and macOS.
  `.github/workflows/asan.yml` additionally builds with Clang and AddressSanitizer and runs the test suite under it.
