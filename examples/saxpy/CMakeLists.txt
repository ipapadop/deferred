add_executable(saxpy_valarray saxpy_valarray.cpp)
target_compile_options(saxpy_valarray
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
      -Wall -Wextra -Wpedantic>)
target_link_libraries(saxpy_valarray
  PRIVATE
    deferred)

add_executable(saxpy_vector saxpy_vector.cpp)
target_compile_options(saxpy_vector
  PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
      -Wall -Wextra -Wpedantic>)
target_link_libraries(saxpy_vector
  PRIVATE
    deferred)
