set(SOURCES
  conditional.cpp
  constant.cpp
  expressions.cpp
  invoke.cpp
  main.cpp
  make_deferred.cpp
  operators.cpp
  variable.cpp)

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
