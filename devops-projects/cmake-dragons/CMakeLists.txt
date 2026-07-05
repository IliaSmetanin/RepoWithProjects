cmake_minimum_required(VERSION 3.11)
set(ENV{CC} "gcc")
set(ENV{CXX} "g++")
project(train_your_dragon)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "C standard 17")
message(STATUS "C++ standard 17")

option(DAY "Day battles" OFF)
option(NIGHT "Night battles" OFF)

enable_testing()

set(CODECOV_URL "https://raw.githubusercontent.com/akhtyamovpavel/CodeCoverageCMakeExample/refs/heads/master/cmake/CodeCoverage.cmake")
set(CODECOV_FILE "${CMAKE_SOURCE_DIR}/cmake/CodeCoverage.cmake")

message("Downloading CodeCoverage.cmake...")
file(DOWNLOAD ${CODECOV_URL} ${CODECOV_FILE})

set(CTEST_COVERAGE_COMMAND "gcov-14")

# make SETUP_TARGET_FOR_COVERAGE available
include(cmake/CodeCoverage.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

add_subdirectory(train)
add_subdirectory(drachenland)
add_subdirectory(arena)

# file running during configuration, COMMAND during make "target"
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/coverage")
SETUP_TARGET_FOR_COVERAGE(
    coverage # target name
    Battle # tested object
    ../coverage # dir for report; .. - because it is related to CMAKE_BINARY_DIR
)

find_package(Python COMPONENTS Interpreter REQUIRED)

add_custom_target(
    lint ALL

    COMMAND
    Python::Interpreter
    "${train_your_dragon_SOURCE_DIR}/utils/includeLint.py"
    -d "${train_your_dragon_SOURCE_DIR}"

    DEPENDS
    "${train_your_dragon_SOURCE_DIR}/utils/includeLint.py"

    COMMENT "Executing linter"
)

# ${CMAKE_COMMAND} let use cross-platform commands (cmake -E)
set(SOURCE_BUILD_DIR "${CMAKE_SOURCE_DIR}/${BUILD_SOURCE_DIR}")
add_custom_target(
    clear
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning build artifacts..."

    COMMAND ${CMAKE_COMMAND} -E rm -rf "${SOURCE_BUILD_DIR}"
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_SOURCE_DIR}/cmake/CodeCoverage.cmake"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_SOURCE_DIR}/coverage"
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_SOURCE_DIR}/coverage.info"
    COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_SOURCE_DIR}/coverage.info.cleaned"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_SOURCE_DIR}/results"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_SOURCE_DIR}/drachenland/lib"
    
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning completed."
    COMMENT "Cleaning build directories, downloaded modules, coverage reports and results"
)