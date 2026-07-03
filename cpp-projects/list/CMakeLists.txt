add_executable(test_fuzzy ${CMAKE_CURRENT_SOURCE_DIR}/test/fuzzy.cpp)
target_link_libraries(test_fuzzy PRIVATE gtest_main pthread)

add_executable(test_list ${CMAKE_CURRENT_SOURCE_DIR}/test/test_list.cpp)
target_link_libraries(test_list PRIVATE gtest_main pthread)

add_executable(test_list_move ${CMAKE_CURRENT_SOURCE_DIR}/test/test_list_move.cpp)
target_link_libraries(test_list_move PRIVATE gtest_main pthread)

add_executable(test_allocator ${CMAKE_CURRENT_SOURCE_DIR}/test/test_allocator.cpp)
target_link_libraries(test_allocator PRIVATE gtest_main pthread)

add_executable(test_benchmark ${CMAKE_CURRENT_SOURCE_DIR}/test/benchmark.cpp)
target_link_libraries(test_benchmark PRIVATE gtest_main pthread)

include_directories(PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/test)
