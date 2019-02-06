set(SOURCES
  axpy.cpp)

add_executable(axpy ${SOURCES})

target_compile_options(axpy
  PRIVATE
    -Wall -Wpedantic -Werror)

target_link_libraries(axpy
  PRIVATE
    deferred)
