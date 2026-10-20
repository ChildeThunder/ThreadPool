cmake_minimum_required(VERSION 3.16)

project(MiniInferenceServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(MiniInferenceServer
    src/main.cpp
    src/ThreadPool.cpp
    src/MockModel.cpp
)

target_include_directories(
    MiniInferenceServer PRIVATE include
)

target_link_libraries(
    MiniInferenceServer PRIVATE Threads::Threads
)

