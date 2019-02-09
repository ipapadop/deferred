set(SOURCES
  constant.cpp
  constant_expressions.cpp
  expressions.cpp
  invoke.cpp
  main.cpp
  operators.cpp
  variable.cpp
  variable_expressions.cpp)

add_executable(test_deferred ${SOURCES})

target_compile_options(test_deferred
  PRIVATE
    -Wall -Wpedantic -Werror)
target_link_libraries(test_deferred
  PRIVATE
    deferred
    Catch2::Catch2)

include(CTest)
add_test(
  NAME
    TestDeferred
  COMMAND
    test_deferred)
