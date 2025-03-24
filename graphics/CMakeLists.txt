cmake_minimum_required(VERSION 3.13.0)
project(geometry VERSION 0.2.0)
set(NAME_EXE geometry)


# linux:
if (UNIX)
message("UNIX configuration")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})

add_executable(geometry main_geom.cpp)

target_link_libraries(geometry ${OPENGL_LIBRARIES})
target_link_libraries(geometry ${GLEW_LIBRARIES})
target_link_libraries(geometry glfw)


# Windows:
else()
message("WIN32 configuration")

set( DIR_SCANNER_LIBS $ENV{PRESCANNING_LIBS_DIR} )

# glfw
include_directories(${DIR_SCANNER_LIBS}/glfw/include/)
link_directories(${DIR_SCANNER_LIBS}/glfw/lib/)

# glew
include_directories(${DIR_SCANNER_LIBS}/glew/include/)
link_directories(${DIR_SCANNER_LIBS}/glew/lib/)

add_executable( ${PROJECT_NAME} main_geom.cpp )

# link
target_link_libraries(${NAME_EXE} glfw3)
target_link_libraries(${NAME_EXE} glew32)
target_link_libraries(${NAME_EXE} OpenGL32)

endif()
