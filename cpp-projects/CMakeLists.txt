cmake_minimum_required(VERSION 3.11)

project(cpp-projects)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# since 3.11 exactly
enable_testing()
# just pulling version with hash
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# make dependency available for downloading and configuration
FetchContent_MakeAvailable(googletest)

add_subdirectory(biginteger)
add_subdirectory(rational)
add_subdirectory(geometry)