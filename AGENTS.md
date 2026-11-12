# Agent Context: Deferred Evaluation Library

This project is a C++23 header-only library for creating deferred evaluation expressions.

## Project Overview

- **Purpose**: Provides a mechanism to define expressions (using constants, variables, and operators) that are evaluated lazily at a later point.
- **Control flow**: `if_`, `switch_`, `while_`, `do_` and `for_` are built by chaining:
  `if_(c).then_(a).else_if_(c2).then_(b).else_(d)`, `switch_(c).case_(l).then_(b).default_(d)`, `while_(c).do_(b)`,
  `do_(b).while_(c)`, `for_(init, c, step).do_(b)`. An `if_` without `else_` and a `switch_` without
  `default_` evaluate to a `std::optional`, or to `void` when every branch produces no value.
- **Loops**: loop constructs evaluate to `void` and may be evaluated more than once; `for_` re-runs its `init`
  clause on every evaluation.
- **`for_` is a flat node**: `for_expression` holds its four clauses directly and runs the loop itself. It was
  briefly a `seq`+`while_` composition; that cost extra moves per construction and leaked those nodes into every
  visitor, for no reuse worth having. `test/unit/for.cpp`'s "a loop outlives the builder" needs ASan to catch a
  loop that refers into its builder.
- **Assignment**: `variable::operator=` is overloaded on the right-hand side. A *value* is stored immediately; a
  *deferred expression* returns an `assign_expression` that assigns when evaluated, which is what makes
  `for_(i = constant(0), i < 10, ++i)` work without a lambda. `i = j` between variables therefore assigns by
  value, re-reading the right-hand side on every evaluation. The deferred form is `[[nodiscard]]` and
  `&`-qualified.
- **An immediate assignment is not an expression**: `v = value` returns `detail::immediate_assignment`, an empty
  tag, not `variable&`. Returning the variable let `if_(c).then_(i = 0)`, `seq(flag = true, ...)` and
  `for_(i = 0, ...)` compile as expressions that only read the variable, after the assignment had already
  happened (#61). `detail::deduce_deferred_type` — behind `make_deferred_t`, which every construct uses —
  has a `static_assert` that rejects the tag and names the fix, `v = constant(value)`. It is a hard error, not
  a failed constraint, so it cannot be pinned by a unit test; check it by hand when touching it. Consequently
  `(i = 5)()` and `a = b = 5` do not compile. Making `=` always deferred was tried and rejected: `v = 10;` as a
  statement then does nothing.
- **Assignment reads a scalar right-hand side first**: `detail::assigned_value()` copies a scalar before the
  target is evaluated, as a built-in assignment does, so `(i = constant(5)) += i` adds the old `i`. Any other
  type keeps the reference `evaluate_ref()` yields, as an overloaded operator's reference parameter would, and a
  prvalue is moved into the target.
- **Assignment from a variable is lvalue-only**: `i = j` works, `i = std::move(j)` and `i = std::as_const(j)`
  are rejected, because copy and move assignment are deleted and, as non-templates, win over the deferred
  `operator=`. `test/unit/assign.cpp` pins it and
  explains why.
- **Compound assignment**: `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=` are deferred free operators
  in `operators.hpp` requiring a `deferred_expression` left operand. They reuse `assign_expression` with an assigning
  operation, so the target is evaluated once and the type's own compound operator is used — they do not expand to
  `t = t + u`. Unlike `=`, they have no eager meaning, so they follow the same rule as every other operator.
- **Short-circuiting**: `&&` and `||` build a `logical_expression`, not an `expression_`; they yield `bool` and
  evaluate the right operand only when the left does not decide the result. Operands that do not convert to
  `bool` (`std::valarray` and expression types like it), or whose types overload the operator themselves (a
  three-valued boolean), take a second, eager overload that calls the type's own operator — which is what C++
  does for an overloaded `&&` too. `detail::short_circuits` decides between the two, using `condition`.
- **Assignment targets must yield an lvalue**: `assign()` and the compound assignments are constrained on
  `detail::assignable_with` / `lvalue_expression` (see `assign.hpp` and `concepts.hpp`). Dropping the constraint reintroduces
  a dangling reference that only ASan catches.
- **Operations whose result may refer into their operand** — prefix `++`/`--`, unary `*` and `subscript()` —
  return `detail::operand_result_t` (see `operators.hpp`): a reference when the operand is an lvalue or a raw
  pointer, and a decayed value otherwise. So `++i` yields `int&` like `i += 1`, `*(p + 1)` yields a reference,
  and `*invoke(make_unique)` yields a copy. Returning `decltype(auto)` from these function objects reintroduces
  a dangling reference that only ASan catches.
- **`[[nodiscard]]` on evaluation**: only `constant` and `variable` have it on `operator()`. Every other node
  may be evaluated for effect, so neither its `operator()` nor `evaluate()`, `evaluate_ref()` and
  `recursive_evaluate()` are `[[nodiscard]]`. Constructing a `constant` or a `variable`, the free factories such
  as `invoke()` and `seq()`, and builder methods all are.
  `test/unit/discarded_evaluation.cpp` turns a discarded result into an error to pin this.
- **`constant` and `variable` are class templates with deduction guides**, not factory functions over
  `constant_`/`variable_` (#53). `constant(42)`, `constant{42}`, `variable(42)` and `variable<int>()` all
  construct the class, so call sites written for the old factories compile unchanged; only the spelled type
  names lost their underscore. A namespace cannot hold a class template and a function template of the same
  name, so the factories are gone and must not be reintroduced.
- **How they are constructed**: two constructors store the argument (`std::type_identity_t<T> const&` and
  `&&`), and one constructor template evaluates it. The template is chosen only when the argument is callable
  with no arguments *and* its evaluated result converts to `T` (`detail::evaluates_to`); it stores
  `recursive_evaluate(u)`, as the old factories did. So `constant([] { return 1; })` holds `1`, while
  `constant<std::function<int()>>{f}` and a `constant<F>` built from an `F` hold the callable.
- **Why the constructors have this shape** — each point was a regression found in review, pinned in
  `test/unit/constant.cpp` and `test/unit/variable.cpp`:
  - A value is stored directly, never through `recursive_evaluate`, which returns a decayed copy; a
    `constant<std::string_view>` built from an lvalue `std::string` would otherwise view a dead temporary.
  - An argument that is evaluated goes through `detail::evaluate_for_node`, which returns an lvalue result
    as that reference and otherwise does what `recursive_evaluate` does. So
    `variable<std::string_view> view{string_variable}` views the string the source holds, where it used to
    view a decayed temporary copy. `detail::evaluates_to` asks about the same type, so
    `variable<std::span<int>>` can be built from a `variable<std::vector<int>>`.
  - `detail::evaluates_to` asks `std::invocable` before forming `node_result_t`. The return type of
    `recursive_evaluate` is deduced, so forming it for an lvalue that cannot be copied is a hard error;
    `std::is_constructible_v<constant<int>, std::mutex&>` must be `false`, not ill-formed.
  - The template excludes the class's own type, leaving it to the copy and move constructors. `std::tuple`
    asks whether every stored node is constructible from an lvalue of itself, and for a
    `constant<std::unique_ptr<int>>` the template would turn that question into an error.
  - The storing parameters are `std::type_identity_t<T>`, so their implicit deduction guides cannot deduce.
    A plain `T const&` guide is more specialized than the user guide and would make
    `auto const f = [] { return 1; }; constant(f)` store the lambda. The same parameters keep
    `variable<std::vector<int>>({1, 2, 3})` working.
  - `T` cannot be a reference (`static_assert`). `constant<int const&>(i)` used to name the factory's
    *argument* type and now names the stored type.
- **The deduction guides** are `constant(T&&) -> constant<detail::node_result_t<T&&>>` and the same for
  `variable`: the decayed, evaluated type, so no guide retains a reference.
- **`variable` cannot be copied or moved**: its copy and move constructors are deleted. `variable w{v}` works
  for a non-const lvalue `v`, through the constructor template, and makes a new, independent variable; the
  template admits its own type only as `variable&`, and only when `T` is copyable. A const or temporary source
  selects the deleted constructor, which as a non-template wins; `variable(v())` and `variable(std::move(v)())`
  are the spellings. Explicit copy and move constructors were tried and rejected: the move let a temporary
  variable live inside a node, so `int& r = (++variable(0))();` dangled, and the copy let
  `invoke([=] { return v(); })` hold a silent snapshot of `v`. Do not reintroduce either.
  One gap remains, the price of `variable w{v}`: a by-copy capture direct-initializes, so a bare
  `[=] { return v(); }` over a non-const `v` compiles and snapshots it. The closure cannot be copied or moved,
  so it cannot be passed to `invoke()` or stored in a node (#73).
- **A node stores every operand by value** (#26). `make_deferred_t` never yields a reference: a named
  constant or expression is copied into the node, a temporary is moved in, and a named variable becomes a
  `node_ref` — `variable_ref<T>`, or `variable_cref<T>` for a const variable — which is a pointer to it. So an
  expression can be copied, returned and kept in a container; only its variables must outlive it. They used
  to be held as `X&`, and `auto c2 = c * 2; return c2;` dangled.
  - `node_ref<Node>` (`node_ref.hpp`) is the one node that refers to another. `variable_ref<T>` and
    `variable_cref<T>` are aliases for `node_ref<variable<T>>` and `node_ref<variable<T> const>`, and
    `std::ref(x)` / `std::cref(x)` on any node become `node_ref<X>` / `node_ref<X const>`, so
    `std::ref(v) + 1` and `v + 1` have the same type. It was three mechanisms before: two near-identical
    classes and an `expression_` over a `std::reference_wrapper` with a special case in
    `expression_::visit`. `variable<T const>` is a distinct, valid variable, so its reference is
    `node_ref<variable<T const>>`, not the const form.
  - The header forward-declares `variable`, so `expression.hpp` can include it. A `node_ref` evaluates and
    visits the node it refers to, and has `get()` when that node has. `detail::visits_tree` is
    specialized so that a visitor is checked against, and shown, the node; `subexpressions()` does hand
    back the `node_ref` (#72).
  - **A temporary `expression_` yields a value where a stored one would yield a reference into itself**
    (`operator()() &&`, and `eval()` likewise). Whether a result is such a reference is read from the
    *type* the operator would yield if the operands were evaluated as temporaries: an owned `constant`
    then yields a value, so `operand_result_t` gives a decayed type, while a `node_ref` still gives a
    reference. The operands are nevertheless evaluated as stored, and the result is copied when the two
    types disagree. Without this, `auto const& s = subscript(table, i)();` over a named constant refers
    into the temporary node's copy. Actually evaluating the operands as temporaries was the first version
    and was wrong twice: it moved every owned value out on each evaluation (a full copy for an array),
    and `invoke(f, constant(s))()` with an `f` that returns a reference to its argument returned one to a
    value already destroyed. Only `expression_` needs the overload: every other node returns through
    `evaluate()`, which decays, or yields the target of an assignment.
  - `detail::callable_needs_mutable` is `false` without asking when the callable is an empty class. Asking
    instantiates the const evaluation of every subexpression, which was about 12% of the compile time of
    a translation unit made of long chains of named operands.
  - A temporary variable cannot enter an expression, and a named operand that cannot be copied must be
    moved in or passed with `std::ref`. Each has its own `static_assert` in `deduce_deferred_type` that
    names the fix; they used to fail inside `std::tuple`'s constructor. They are hard errors, so no test
    pins them; check the messages by hand.
  - A variable local to a function still cannot be used by an expression the function returns (#71).
  - Rejected, with measurements (GCC 15 and Clang 22, `-O2`, minimum of 9 runs): making `variable` a
    handle to shared state, so that it too is owned. Linked handles with no heap, and a heap count, both
    passed ASan, but cost up to 6x on a counter-and-sum `for_` loop and 11x on a saxpy loop over
    `subscript()`, against no measurable change for `variable_ref`. A stored pointer to a separate named
    object optimizes as the reference did; shared state reached through handles did not.
  - `test/unit/node_ref.cpp` and `test/integration/ownership.cpp` pin this; the latter needs ASan to
    catch a regression.
- **A named operand that owns state is a compile error**: `detail::deduce_deferred_type` has a
  `static_assert` for an lvalue operand for which `detail::owns_state` holds. The message names the two
  fixes: `std::ref(x)` to refer to it, `std::move(x)` to hand it over. Copying it silently was the first
  behaviour of by-value storage and was rejected: a user node with `mutable_state` lost its writes, and a
  named `invoke([n = 0]() mutable { ... })` used in two expressions ran two counters.
  - `owns_state` is true for a node with `mutable_state` that does not merely refer to state
    (`detail::refers_to_state`: `node_ref`, `assign_expression`), for an `expression_`
    whose callable cannot be called as const, and for any node that stores one of those. The callable is
    asked last, because asking evaluates the subexpressions as const.
  - Like the `immediate_assignment` check it is a hard error, so no test pins it; check it by hand (#74). Only an
    *lvalue* operand is checked. `conditional_builder::then_() const&` therefore copies its stored condition
    explicitly rather than passing the member, which would present it as a named operand.
  - `std::ref(x)` / `std::cref(x)` are stored as a `node_ref`. A `std::reference_wrapper` to something
    that is not a node is still an ordinary callable. The `deferred_operand` concept lets the unary
    operators, compound assignments, `subscript()` and `assign()` accept one, and `any_deferred` is defined
    over it.
  - A user node outside `namespace deferred` is not found by the operators through ADL, with or without
    `std::ref`; such code needs `using namespace deferred`.
- **Lazy operand types and temporaries**: an operator on `std::valarray` may return a proxy that refers to its
  operands. A node evaluates its operands inside its own call, so a proxy over a *temporary* operand —
  `invoke([] { return std::valarray<float>(...); }) * constant(2.0f)` — dangles once the node returns, where
  the plain C++ statement would be fine (#68). An operand held in a `variable` is safe, because the node
  refers to it. An operand held in a named `constant` is copied into the node and is safe for as long as the
  node lives; a *temporary* node over such constants, such as the one a body returns in
  `if_(c).then_([&] { return dx + dy; })`, yields a proxy into copies that die with it. libstdc++ has held
  nested proxies by value since GCC 9, which is what fixed #2. `test/integration/operators.cpp` pins the safe
  case.
- **Named access**: `constant` and `variable` have `get()`, and a `node_ref` to one forwards it, a named spelling of `operator()` with the same
  cv/ref-qualified overloads, return types and `noexcept`. It delegates to `operator()`, so the two cannot
  drift apart. No other node has it: `get()` reads a stored value, and the call evaluates an expression.
- **Named evaluation**: every other node — `expression_`, `assign_expression`, `logical_expression`,
  `seq_expression`, `conditional_expression`, `switch_expression` with its `case_expression` and
  `default_expression`, and the three loops — has `eval()`, a named spelling of `operator()`. Each class defines
  a const and a non-const overload that return `(*this)()`, so result type and `noexcept` are the operator's. A
  new node must define both. `expression_` also has an rvalue overload of each. `eval()` is not
  `[[nodiscard]]`, for the same reason the operator is not. `constant`, `variable`, `node_ref` and
  the builders have no `eval()`; `test/unit/eval.cpp` pins all of this.
- **Node interface**: every node has `subexpression_types` and `subexpressions()`, which returns a tuple of
  references in the same order; a named variable appears there as its `node_ref`. `expression_`'s operator is *not* in `subexpression_types`; it is exposed as
  `operator_type` / `operator_()`, and `is_constant_expression` checks it through `operator_type`. Nodes whose
  result does not depend on constness name it `result_type`.
- **Builders construct the last operand in place**: `then_`, `else_` and `default_` forward their argument
  into the node rather than building a branch, case or default and moving it in. `conditional_branch` and
  `case_expression` take a `detail::forwarded_pair` for that, since a `std::tuple` element is constructed
  from one argument. Do not replace it with a `std::tuple` of references: that was the first version, and
  GCC 15 at `-O2` then ran a nested `while_`/`if_` loop 4.5x slower (31 ms against 140 ms; `-O3` and Clang
  were unaffected). Operands are owned now, so an extra move is a move of the value, not of a reference;
  `test/integration/ownership.cpp` pins one copy and no move.
- **Every entry point returns a builder**: `if_`, `switch_`, `while_`, `do_` and `for_` return a type with no
  `operator()` that is not a `deferred_expression`. `switch_builder` accepts `case_` and `default_` only.
- **Both chains are extendable**: `case_` may follow `default_`, and `else_if_` may follow `else_`; the new
  branch is checked after the existing ones and before the default or else. `conditional_builder` carries the
  else through its third template parameter. `else_` and `default_` are accepted once.
- **`evaluate_ref()` vs `evaluate()`**: `evaluate()` returns by value, so it copies a result that is already a
  reference. Positions that only read within one full expression — loop and `if_` conditions, `switch_` case
  labels, discarded bodies and steps, the right-hand side of an assignment, `&&`/`||` operands — use
  `evaluate_ref()`. Anything that stores or
  returns a result keeps `evaluate()`, which must decay.
- **`operator[]` cannot be a free function**, so `deferred::subscript(a, b)` is its spelling. Unary `*` is a normal
  free operator; both require only a `deferred_expression` operand. `->` and unary `&` are deliberately absent: `->` must be a member, and overloading `&` would break
  taking the address of an expression.
- **`for_` accepts a bare variable as a clause**: it has no effect, so `for_(i, i < n, ++i)` resumes from the
  current `i`. `for_` once rejected it with its own `static_assert`, to catch `for_(i = 0, ...)`; the
  `immediate_assignment` tag now catches that everywhere. (`detail::is_variable` exists again, in
  `node_ref.hpp`, for a different purpose: choosing a `node_ref` for a variable operand.)
- **`seq`**: `seq(a, b, ...)` evaluates left to right and yields the last, like the comma operator. It is how a
  loop body, branch or `for_` clause does more than one thing without a lambda.
- **`assign_expression`, `logical_expression` and `seq_expression` cannot be `expression_`s**: `expression_`
  evaluates its subexpressions as the arguments of one call, so it can express neither ordering nor conditional
  evaluation. `assign.hpp` gives the full reason. Do not "simplify" any of them back into an `expression_` over a
  function object.
- **Visitor order**: children are visited in `subexpression_types` order, which is the source order of the
  construct, not its evaluation order.
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
- C++23 capable compiler. Supported means tested in CI: Clang 22, Apple Clang 21, GCC 13.3, MSVC 19.51. The
  project tracks current compilers and does not keep workarounds for older ones. GCC 13.3 is supported, so C++23
  features it lacks, such as deducing `this`, stay out.
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
- **`DEFERRED_NO_UNIQUE_ADDRESS`**: use it, never `[[no_unique_address]]` directly. The Microsoft ABI cannot
  change existing layouts, so MSVC and clang-cl accept the standard attribute but ignore it and provide
  `[[msvc::no_unique_address]]`. `detail/no_unique_address.hpp` picks by attribute detection rather than by
  compiler macro, because clang-cl reports `__clang__` yet needs the Microsoft spelling. This is the only macro in
  the library besides header guards.
- **Concept names**: concepts are lower-case with underscores, as in the standard library
  (`deferred_expression`, `deferred_operand`, `any_deferred`, `has_subexpression_types`, `has_mutable_state`,
  and everything in
  `detail`). The earlier capitalized spellings (`Deferred`, `AnyDeferred`, ...) were removed without a
  compatibility period; do not reintroduce them.
- **Public concepts and where they live**: `deferred_expression`, `deferred_operand`, `any_deferred` and
  `has_subexpression_types`
  are in `type_traits/is_deferred.hpp`; `has_mutable_state` and `constant_expression` in
  `type_traits/is_constant_expression.hpp`; `expression_visitor` in `type_traits/expression_visitor.hpp`;
  `expression_yielding`, `lvalue_expression`, `condition` and `case_label_for` in `concepts.hpp`, which needs
  `make_deferred_t` and so cannot sit under `type_traits/`.
- **Entry points are constrained**: `if_`, `else_if_`, `while_`, `do_(...).while_` and the condition of `for_`
  require `condition`; both `case_` overload sets require `case_label_for` the switch condition; every node's
  `visit()` requires `expression_visitor` for that node. `condition` and `case_label_for` take the *argument*
  type and apply `make_deferred_t` themselves, so plain values and callables pass. A test that builds one of
  these constructs only to check `noexcept` still needs a type that satisfies the concept.
- **`expression_visitor` checks the whole tree**, through `detail::visits_tree` over `subexpression_types`, the
  same walk `detail::is_nothrow_visitable` makes. Checking only the visited node would move the failure into a
  child's `visit()` inside the parent's `noexcept` specifier, which is a hard error, not a rejected overload.
- **`condition` is also the name of many function parameters**. Use it as a type-constraint in the template
  head (`template<condition C>`); in a trailing `requires` it would name the parameter.
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
  `.github/workflows/asan.yml` additionally builds with Clang 22, installed from apt.llvm.org, and AddressSanitizer and runs the test suite under it.
