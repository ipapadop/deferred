set(SOURCES
  apply.cpp
  conditional.cpp
  constant.cpp
  main.cpp
  switch.cpp
  variable.cpp)

add_executable(deferred_integration_tests ${SOURCES})

target_compile_options(deferred_integration_tests
  PRIVATE
    -Wall -Wpedantic -Werror)
target_link_libraries(deferred_integration_tests
  PRIVATE
    deferred
    Catch2::Catch2)

include(CTest)
add_test(
  NAME
    IntegrationTests
  COMMAND
    deferred_integration_tests)
