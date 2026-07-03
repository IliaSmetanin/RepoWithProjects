file(GLOB_RECURSE TESTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(test_geometry "${TESTS}" "${SOURCES}")

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories(test_geometry PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_geometry PRIVATE gtest_main pthread)