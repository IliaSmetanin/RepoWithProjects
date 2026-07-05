cmake_minimum_required(VERSION 3.11)
project(BATTLE)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${BATTLE_SOURCE_DIR}/../cmake/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# find_package(DRAGONS REQUIRED)
# message("Lib Dragons has been found!")

# since 3.11 exactly
include(FetchContent)

# just pulling version with hash
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# make dependency available for downloading and configuration
FetchContent_MakeAvailable(googletest)

option(DAY "Set the day for battle" OFF)
option(NIGHT "Set the night for battle" OFF)

if (DAY)
    add_compile_definitions(day)
elseif (NIGHT)
    add_compile_definitions(night)
endif()

add_executable(Battle battle.cpp)

# file(GLOB_RECURSE Dragons_HEADERS "${BATTLE_SOURCE_DIR}/../drachenland/*y")
set(
    BATTLE_HEADERS
    "${BATTLE_SOURCE_DIR}/../drachenland/icy"
    "${BATTLE_SOURCE_DIR}/../drachenland/fury"
    "${BATTLE_SOURCE_DIR}/../drachenland/fiery"
    "${BATTLE_SOURCE_DIR}/../utils"
)

target_include_directories(Battle PRIVATE "${BATTLE_HEADERS}")
# gtest_main - library that substitute function main that launch all tests
target_link_libraries(Battle PRIVATE Dragons gtest_main pthread)

enable_testing()
include(GoogleTest)

# instead of add_test(NAME "smth.smth" COMMAND --gtest_filter=...)
# get --gtest_list_tests after building Battle
# DISCOVERY_MODE PRE_TEST - not to launch test binary file on configuration stage
gtest_discover_tests(Battle DISCOVERY_MODE PRE_TEST)
message("Tests enabled!")

message("Executable \"Battle\" has been built!")