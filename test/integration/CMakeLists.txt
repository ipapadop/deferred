set(SOURCES
  apply.cpp
  conditional.cpp
  constant.cpp
  main.cpp
  operators.cpp
  switch.cpp
  variable.cpp
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
