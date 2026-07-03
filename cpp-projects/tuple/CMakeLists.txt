add_executable(test_tuple ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
target_link_libraries(test_tuple PRIVATE gtest_main pthread)