#
# Catch2
#
if((NOT CATCH_INCLUDE_DIR) OR (NOT EXISTS ${CATCH_INCLUDE_DIR}))
  execute_process(
    COMMAND
      git submodule update --init -- external/Catch2
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR})

  set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/Catch2
      CACHE PATH "Catch2 directory")
endif()

add_subdirectory(Catch2 ${CMAKE_CURRENT_BINARY_DIR}/Catch2)
