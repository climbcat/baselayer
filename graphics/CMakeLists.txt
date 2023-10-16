cmake_minimum_required(VERSION 3.0.0)
project(geometry VERSION 0.1.0)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})

add_executable(geometry main_geom.cpp)

target_link_libraries(geometry ${SDL2_LIBRARIES})
target_link_libraries(geometry ${OPENGL_LIBRARIES})

