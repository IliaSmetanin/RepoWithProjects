cmake_minimum_required(VERSION 3.11)

project(TRAIN)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_C_STANDARD 17)
#set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES
    train.cpp
    trench.cpp
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TRAIN_SOURCE_DIR}/../results)
add_executable(train ${SOURCES})

message("Executable \"train\" has been built!")
