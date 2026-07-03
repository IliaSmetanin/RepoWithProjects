add_executable(test_deque test.cpp "${SOURCES}")

target_include_directories(test_deque PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_deque PRIVATE bigint gtest_main pthread)