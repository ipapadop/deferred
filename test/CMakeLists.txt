include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.13.0
)

FetchContent_MakeAvailable(Catch2)

add_subdirectory(unit)
add_subdirectory(integration)
