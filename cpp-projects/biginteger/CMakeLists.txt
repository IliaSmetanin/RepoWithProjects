file(GLOB_RECURSE TESTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(test_biginteger "${TESTS}" "${SOURCES}")

target_include_directories(test_biginteger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_biginteger PRIVATE gtest_main pthread)