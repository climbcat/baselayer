cmake_minimum_required(VERSION 3.0.0)
project(memory_experiments VERSION 0.1.0)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(memory main.cpp)

target_link_libraries(memory ${SDL2_LIBRARIES})
