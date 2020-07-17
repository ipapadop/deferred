cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(deferred
  VERSION 1.0.0
  LANGUAGES CXX)

# dependencies

add_subdirectory(external)

# targets and properties

add_library(deferred INTERFACE)
add_library(deferred::deferred ALIAS deferred)
target_include_directories(deferred
  INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(deferred INTERFACE cxx_std_17)

# installation

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/deferred)

install(TARGETS deferred
  EXPORT deferred-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT deferred-targets
  FILE
    deferred.cmake
  NAMESPACE
    deferred::
  DESTINATION
    ${INSTALL_CONFIGDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/deferredConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/deferredConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/deferredConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/deferredConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/deferredConfigVersion.cmake
  DESTINATION ${INSTALL_CONFIGDIR})

# documentation

find_package(Doxygen)
option(DEFERRED_BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(DEFERRED_BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/doc/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(documentation
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()

# examples

add_subdirectory(examples)

# testing

include(CTest)
enable_testing()
add_subdirectory(test)
