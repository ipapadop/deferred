set(SOURCES
  conditional.cpp
  constant.cpp
  invoke.cpp
  main.cpp
  make_function_object.cpp
  switch.cpp
  variable.cpp
  while.cpp)

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
