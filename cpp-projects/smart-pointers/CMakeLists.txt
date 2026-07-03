add_executable(test_shared ${CMAKE_CURRENT_SOURCE_DIR}/test/test_shared.cpp)
target_link_libraries(test_shared PRIVATE gtest_main pthread)

add_executable(test_weak ${CMAKE_CURRENT_SOURCE_DIR}/test/test_weak.cpp)
target_link_libraries(test_weak PRIVATE gtest_main pthread)

add_executable(test_esft ${CMAKE_CURRENT_SOURCE_DIR}/test/test_esft.cpp)
target_link_libraries(test_esft PRIVATE gtest_main pthread)

include_directories(PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
