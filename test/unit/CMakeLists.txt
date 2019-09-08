set(SOURCES
  conditional.cpp
  constant.cpp
  invoke.cpp
  main.cpp
  switch.cpp
  variable.cpp
  while.cpp)

add_executable(unit_tests ${SOURCES})

target_compile_options(unit_tests
  PRIVATE
    -Wall -Wpedantic -Werror)
target_link_libraries(unit_tests
  PRIVATE
    deferred
    Catch2::Catch2)

include(CTest)
add_test(
  NAME
    UnitTests
  COMMAND
    unit_tests)
