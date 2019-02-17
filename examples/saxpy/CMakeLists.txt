add_executable(saxpy_valarray saxpy_valarray.cpp)

target_compile_options(saxpy_valarray
  PRIVATE
    -Wall -Wpedantic -Werror)

target_link_libraries(saxpy_valarray
  PRIVATE
    deferred)

add_executable(saxpy_vector saxpy_vector.cpp)

target_compile_options(saxpy_vector
  PRIVATE
    -Wall -Wpedantic -Werror)

target_link_libraries(saxpy_vector
  PRIVATE
    deferred)
