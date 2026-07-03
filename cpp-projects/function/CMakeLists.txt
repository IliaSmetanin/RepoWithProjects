add_executable(test_function ${CMAKE_CURRENT_SOURCE_DIR}/test/test_function.cpp)
target_link_libraries(test_function PRIVATE gtest_main pthread)

add_executable(test_move_only_function ${CMAKE_CURRENT_SOURCE_DIR}/test/test_move_only_function.cpp)
target_link_libraries(test_move_only_function PRIVATE gtest_main pthread)

include_directories(PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
