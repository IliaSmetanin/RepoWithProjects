cmake_minimum_required(VERSION 3.11)
project(DRAGONS)

# not necessary with INSTALL
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${DRAGONS_SOURCE_DIR}/lib")

file(GLOB_RECURSE SOURCES "${DRAGONS_SOURCE_DIR}/*.cpp")

message("Building library \"Dragons\"...")

add_library(Dragons SHARED "${SOURCES}" "${DRAGONS_SOURCE_DIR}/fury/toothless.c")

if(DAY)
    target_compile_definitions(Dragons PRIVATE day)
elseif(NIGHT)
    target_compile_definitions(Dragons PRIVATE night)
endif()

target_include_directories(Dragons PRIVATE "${DRAGONS_SOURCE_DIR}/../utils")

message("Library \"Dragons\" has been built")

file(GLOB_RECURSE LIB_HEADERS "${DRAGONS_SOURCE_DIR}/*.hpp")
set(LIB_HEADERS "${LIB_HEADERS}" "${DRAGONS_SOURCE_DIR}/../utils")
set_target_properties(Dragons PROPERTIES PUBLIC_HEADER "${LIB_HEADERS}")

# INSTALL(
#     TARGETS Dragons
#     LIBRARY DESTINATION "${DRAGONS_SOURCE_DIR}/lib"
#     PUBLIC_HEADER DESTINATION "${DRAGONS_SOURCE_DIR}/include"
# )