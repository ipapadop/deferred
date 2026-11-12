# deferred

Library for creating deferred evaluation expressions in C++23.

``deferred`` provides:
- functions to declare constants and variables,
- functions to create deferred evaluation expressions from functions,
- deferred ``if``, ``switch``, ``while``, ``do``-``while`` and ``for`` built from chained
  expressions,
- deferred assignment to variables, including compound assignment,
- ``deferred::seq`` for sequencing several expressions where one is expected,
- ``deferred``-enabled commonly used operators, with short-circuiting ``&&`` and ``||``.

Requirements
------------

- C++23 capable compiler.
- CMake 3.28.1 and higher.

Compilers tested, which are the ones supported:
- Clang 22 (Ubuntu, under AddressSanitizer)
- Apple Clang 21 (macOS)
- GCC 13.3 (Ubuntu)
- MSVC 19.51 (Windows)

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

Constants and variables
-----------------------

``deferred::constant`` and ``deferred::variable`` are class templates whose type
is deduced from the value they are built with. Both spellings below are the
same thing:

```C++
deferred::constant c{42};            // constant<int>
auto d = deferred::constant(42);     // constant<int>

deferred::variable v{42};            // variable<int>
deferred::variable<int> w;           // no initial value, so the type is named
```

A constant or a variable owns its value. The deduced type is the decayed type
of the argument, so a reference is never retained and an array is held as a
pointer:

```C++
int i = 1;
deferred::constant c{i};        // constant<int>, a copy of i
deferred::constant s{"text"};   // constant<char const*>
```

An argument that can be called with no arguments is called, and the result is
stored. This repeats until the result is not callable, and applies to a
deferred expression too:

```C++
deferred::constant c{[] { return 1; }};   // constant<int> holding 1
deferred::variable v{c + c};              // variable<int> holding 2
```

So neither can hold a nullary callable such as a ``std::function<int()>``.

A constant can be copied. A variable cannot be copied or moved, but it is an
expression like any other, so a new variable can start from its value:

```C++
deferred::variable w{v};     // a new, independent variable with the current value of v
deferred::variable x{v()};   // the same; required when v is const or a temporary
```

Earlier releases named the class templates ``constant_`` and ``variable_`` and
provided ``constant()`` and ``variable()`` as factory functions. Calls such as
``deferred::constant(42)`` and ``deferred::variable<int>()`` compile unchanged,
because they now construct the class. Code that named the types must drop the
trailing underscore: ``constant_<int>`` is now ``constant<int>``.

Control flow
------------

Conditionals, switches and loops are built one piece at a time; every construct
alternates between a pending builder and a complete expression:

```C++
auto ex = deferred::if_(cond1).then_(a)
            .else_if_(cond2).then_(b)
            .else_(c);

auto s = deferred::switch_(var)
           .case_(10).then_([] { return "10"; })
           .case_(12).then_([] { return "12"; })
           .default_("unknown");

auto loop = deferred::while_(n != 0).do_(--n);

auto at_least_once = deferred::do_(--n).while_(n != 0);

auto counted = deferred::for_(i = deferred::constant(0), i < 10, ++i).do_(body);
```

Loops evaluate to ``void``, and a loop expression may be evaluated more than
once: ``for_`` re-runs its initialization on every evaluation rather than once
at construction.

Assigning to a variable has two meanings, chosen by the right-hand side.
Assigning a *value* stores it immediately; assigning a *deferred expression*
instead builds an expression that assigns when it is evaluated, which is what
lets a loop clause, a body or a branch assign:

```C++
n = 10;                                   // assigns now
auto init = (i = deferred::constant(0));  // assigns nothing yet
init();                                   // now i is 0
```

Because the deferred form looks like an ordinary assignment, it is
``[[nodiscard]]`` -- writing ``i = deferred::constant(0);`` as a statement warns
rather than silently doing nothing -- and it only binds to a named variable, so
it cannot capture a temporary.

A variable is itself a deferred expression, so ``i = j`` assigns one variable to
another. It follows ordinary C++ assignment: the target receives a copy of the
value the right-hand side evaluates to, and the two stay independent afterwards.
The right-hand side is read on every evaluation, not captured when the
expression is built. Only a plain lvalue works as the source:
``i = std::move(j)`` and ``i = std::as_const(j)`` are rejected.

An immediate assignment is not an expression. Its result is a tag that nothing
accepts, so an assignment that has already happened cannot be passed where one
that happens later was meant. Each of these is a compile error whose message
says to write ``deferred::constant(value)``:

```C++
auto a = deferred::if_(i > 10).then_(i = 0);                         // rejected
auto b = deferred::while_(n != 0).do_(deferred::seq(seen = true, n -= 1));  // rejected
auto c = deferred::for_(i = 0, i < 5, ++i).do_(body);                // rejected

auto ok = deferred::for_(i = deferred::constant(0), i < 5, ++i).do_(body);
```

For the same reason an immediate assignment yields no value: ``(i = 5)()`` and
``a = b = 5`` do not compile. Write ``a = 5; b = 5;``, or
``a = (b = deferred::constant(5))`` for the deferred form.

A ``for_`` clause may be a bare variable, which has no effect, so a loop can
resume from the current value:

```C++
auto resume = deferred::for_(i, i < 5, ++i).do_(body);   // i starts wherever it is
```

``deferred::assign`` is the free-function form, for targets that are not a
variable. A target must evaluate to an lvalue, since the result refers into it;
an expression yielding a temporary is rejected rather than left dangling:

```C++
auto ex = deferred::assign(deferred::invoke(&counter::value, &c), deferred::constant(42));
```

A variable holding a reference-like type, such as ``std::reference_wrapper``,
does whatever that type's own assignment does -- it rebinds rather than
assigning through.

Compound assignment (``+=``, ``-=``, ``*=``, ``/=``, ``%=``, ``&=``, ``|=``,
``^=``, ``<<=``, ``>>=``) is deferred whenever its left operand is, like every
other operator in the library -- only plain ``=`` carries the eager meaning.
Each uses the target type's own compound operator and evaluates the target once,
rather than expanding to ``t = t + u``.

An ``if_`` chain without ``else_``, and a ``switch_`` without ``default_``, are
usable expressions that return a ``std::optional`` (or nothing, when the branches
return ``void``).

Branch, case and default bodies are evaluated with ``deferred::evaluate``, which
keeps evaluating while the result is itself a deferred expression. A body that
returns a deferred expression therefore contributes the value that expression
evaluates to, and the result of a construct is always a value -- never an
expression type, and never a reference into a subexpression.

A case may be added to a switch expression that already has a ``default_``; it is
checked after the cases already present and before the default:

```C++
auto expanded = s.case_(11).then_([] { return "11"; });
```

An ``if_`` chain is extended the same way: an ``else_if_`` added to a chain that
already has an ``else_`` is checked after the branches already present and
before the else. A chain has a single ``else_``.

```C++
auto base     = deferred::if_(n == 1).then_(10).else_(-1);
auto expanded = base.else_if_(n == 2).then_(20);
```

``if_(c)``, ``switch_(c)``, ``while_(c)``, ``do_(b)`` and ``for_(i, c, s)`` only
start a chain. None of them is an expression until its next piece is supplied:
``then_``, a ``case_`` with its ``then_`` or a ``default_``, ``do_`` or
``while_``.

Sequencing
----------

``deferred::seq`` evaluates several expressions left to right and yields the
last, as the built-in comma operator does. It is what lets a construct that
takes a single expression -- a loop body, a branch, a ``for_`` clause -- do more
than one thing without a lambda:

```C++
auto ex = deferred::while_(n != 0).do_(deferred::seq(ticks += 1, n -= 1));
```

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

Short-circuiting
----------------

``&&`` and ``||`` evaluate their right operand only when the left one does not
already decide the result, as the built-in operators do. Operands that do not
convert to ``bool``, such as ``std::valarray``, or whose types overload the
operator themselves, such as a three-valued boolean, keep that operator's
meaning and result type and are both evaluated -- which is also what C++ does
for an overloaded ``&&``.

```C++
auto ex = n != 0 && (bumps += 1);   // bumps is not touched while n is 0
```

Subscripting and dereferencing
------------------------------

``*`` dereferences a deferred expression. ``operator[]`` can only be a
non-static member function, so it cannot be supplied for every deferred
expression the way the other operators are; ``deferred::subscript`` is its
free-function spelling:

```C++
auto element = deferred::subscript(container, index);
auto pointee = *pointer_expression;
```

Both yield a reference when it cannot dangle: when the operand evaluates to an
lvalue, or to a raw pointer, which does not own what it points to. Any other
operand is a temporary of the evaluation, so the result is returned by value.
Prefix ``++`` and ``--`` follow the same rule, so ``++i`` on a variable yields
the variable, as ``i += 1`` does:

```C++
int& first  = deferred::subscript(vector_variable, 0)();  // refers into the variable
int& second = (*(pointer_variable + 1))();                // a raw pointer owns nothing
int  third  = deferred::subscript(deferred::invoke(make_vector), 2)();  // a copy
```

Evaluating expressions
----------------------

Calling an expression evaluates it. An expression may be evaluated only for its
effect, so discarding the result does not warn, except for a constant or a
variable, whose evaluation has no effect:

```C++
(++i)();                      // fine, like (i += 1)();
deferred::seq(++i, ++j)();    // fine
```

Three functions evaluate an argument that may or may not be a deferred
expression; a plain value is returned unchanged.

- ``deferred::evaluate(x)`` evaluates ``x`` and returns the result by value. If
  the result is itself a deferred expression, it is evaluated once more.
- ``deferred::evaluate_ref(x)`` does the same but keeps a reference result as a
  reference, which avoids a copy when the result is only read.
- ``deferred::recursive_evaluate(x)`` keeps evaluating until the result is not
  callable.

Reading a constant or a variable
--------------------------------

Calling a constant or a variable evaluates it, as for any other expression. Each
also has ``get()``, which returns the same thing under a name. Use ``get()`` when
the code reads or writes the stored value, and the call when the object is
evaluated as an expression:

```C++
auto v = deferred::variable(1);
auto c = deferred::constant(2);

v.get() = 10;                     // writes the variable, like v = 10
int  now   = v.get() + c.get();   // 12
auto moved = std::move(v).get();  // moves the value out
```

``get()`` has the reference and ``noexcept`` qualification of the matching
call: a reference into the object for an lvalue, and a value for an rvalue. Only
``constant`` and ``variable`` have it.

Every other expression has ``eval()`` instead, a named spelling of the call. It
does whatever the call does, side effects included, and has the same result
type and ``noexcept``:

```C++
auto sum = v + c;
int  a   = sum();       // evaluates
int  b   = sum.eval();  // the same
(v += 1).eval();        // evaluated for its effect; discarding the result does not warn
```

The two names are deliberately different: ``get()`` reads a stored value and has
no effect, and ``eval()`` runs an expression. A constant or a variable has no
``eval()``, and the builders that ``if_``, ``switch_``, ``while_``, ``do_`` and
``for_`` return have neither.

Concepts
--------

Concepts are named in lower case, like those of the standard library:

| Concept | Satisfied by |
| --- | --- |
| ``deferred::deferred_expression<T>`` | a deferred expression, ignoring cv-qualifiers and references |
| ``deferred::any_deferred<T...>`` | a pack in which at least one type is a ``deferred_expression`` |
| ``deferred::has_subexpression_types<T>`` | a type that names ``subexpression_types``; cv-qualifiers and references are not ignored |
| ``deferred::has_mutable_state<T>`` | a node whose own stored value can change between evaluations, such as a variable |
| ``deferred::constant_expression<T>`` | a ``deferred_expression`` for which ``is_constant_expression_v`` is true |
| ``deferred::expression_yielding<T, R>`` | a ``deferred_expression`` whose result, when called, converts to ``R`` |
| ``deferred::lvalue_expression<T>`` | a ``deferred_expression`` whose result is an lvalue, such as a variable or ``++i`` |
| ``deferred::condition<T>`` | an argument whose result converts to ``bool``, explicitly or implicitly |
| ``deferred::case_label_for<L, C>`` | an argument whose result can be compared with ``==`` to the result of the switch condition ``C`` |
| ``deferred::expression_visitor<V, N>`` | a visitor that can be called with ``N`` and with every node below it |

```C++
template<deferred::deferred_expression T>
auto twice(T&& t)
{
  return std::forward<T>(t) * deferred::constant(2);
}
```

``condition`` and ``case_label_for`` take the argument as it is passed to the
construct, so a plain value or a callable satisfies them as well as a deferred
expression. The library applies them where they matter, so a mistake is
reported at the call that makes it:

```C++
deferred::if_(deferred::constant(std::string{}));  // error: not a condition
deferred::switch_(i).case_("one");                 // error: an int is not comparable to a string literal
i.visit([](int) { });                              // error: not a visitor for this expression
```

``assign()`` and the compound assignments require an ``lvalue_expression``
target, since their result refers into it.

The traits ``is_deferred`` and ``is_constant_expression``, with their ``_v`` and
``_t`` forms, are unchanged.

Earlier releases spelled these concepts ``Deferred``, ``AnyDeferred``,
``HasSubexpressionTypes`` and ``HasMutableState``. Those names are removed;
code that uses them must switch to the lower-case names.

Inspecting expressions
----------------------

Every node names the types of its subexpressions as ``subexpression_types``, a
``std::tuple``, and returns references to them with ``subexpressions()``, in the
same order. A constant or a variable has none. An ``expression_`` additionally
exposes its operator as ``operator_type`` and ``operator_()``; the operator is
not a subexpression.

A node whose result type does not depend on how it is evaluated names it as
``result_type``: ``void`` for the loops, ``bool`` for ``&&`` and ``||``, and the
common or variant type for ``if_`` and ``switch_``. A constant or a variable
names the type it stores as ``value_type``.

```C++
auto sum = i + deferred::constant(2);
auto const& [left, right] = sum.subexpressions();   // left is i itself
```

``deferred::type_name<T>()`` returns a readable name for a type, which helps
when printing an expression tree.

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
