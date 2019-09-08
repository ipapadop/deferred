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
    -Wall -Wpedantic -Werror)
target_link_libraries(integration_tests
  PRIVATE
    deferred
    Catch2::Catch2)

include(CTest)
add_test(
  NAME
    IntegrationTests
  COMMAND
    integration_tests)
