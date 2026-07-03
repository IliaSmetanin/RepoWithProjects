add_executable(test_unordered ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp)
target_link_libraries(test_unordered PRIVATE gtest_main pthread)

add_executable(test_performance ${CMAKE_CURRENT_SOURCE_DIR}/test/perf.cpp)
target_link_libraries(test_performance PRIVATE gtest_main pthread)

include_directories(PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
