add_executable(trivial main.cpp)

target_compile_options(trivial
  PRIVATE
    -Wall -Wpedantic -Werror)

target_link_libraries(trivial
  PRIVATE
    deferred)
