file(GLOB_RECURSE TESTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(bigint STATIC ${CMAKE_SOURCE_DIR}/biginteger/src/biginteger.cpp)
target_include_directories(bigint PRIVATE ${CMAKE_SOURCE_DIR})

add_executable(test_rational "${TESTS}" "${SOURCES}")

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_include_directories(test_rational PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../biginteger)
target_link_libraries(test_rational PRIVATE bigint gtest_main pthread)