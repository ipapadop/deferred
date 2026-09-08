set(SOURCES
  apply.cpp
  assign.cpp
  conditional.cpp
  constant.cpp
  do_while.cpp
  for.cpp
  main.cpp
  operators.cpp
  seq.cpp
  switch.cpp
  variable.cpp
  visitor.cpp
  while.cpp)

add_executable(integration_tests ${SOURCES})
target_compile_options(integration_tests
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
      -Wall -Wextra -Wpedantic>)
target_link_libraries(integration_tests
  PRIVATE
    deferred
    Catch2::Catch2)

add_test(
  NAME
    IntegrationTests
  COMMAND
    integration_tests)
