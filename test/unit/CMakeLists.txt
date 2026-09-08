set(SOURCES
  apply.cpp
  conditional.cpp
  constant.cpp
  evaluate.cpp
  invoke.cpp
  is_constant_expression.cpp
  is_deferred.cpp
  main.cpp
  switch.cpp
  variable.cpp
  visitor.cpp
  while.cpp
  detail/apply_evaluated.cpp
  detail/callable_storage.cpp
  detail/expression_reference.cpp
  detail/homogenized_type.cpp
  detail/is_nothrow_evaluable.cpp
  detail/is_nothrow_visitable.cpp
  detail/map_result.cpp
  detail/map_void.cpp
  detail/no_match_is_nothrow.cpp
  detail/unique_list.cpp
  detail/unmatched_result.cpp
  detail/visit_children.cpp)

add_executable(unit_tests ${SOURCES})
target_compile_options(unit_tests
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
      -Wall -Wextra -Wpedantic>)
target_link_libraries(unit_tests
  PRIVATE
    deferred
    Catch2::Catch2)

add_test(
  NAME
    UnitTests
  COMMAND
    unit_tests)
