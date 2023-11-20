cmake_minimum_required(VERSION 3.13.0)
project(glfw_test VERSION 0.1.0)

# Linux:
if (UNIX)
message("UNIX configuration")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})

add_executable( ${PROJECT_NAME} main_glfw.cpp )

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glfw)

# Windows:
else()
message("WIN32 configuration")

set( LIB_GLFW glfw )
set( LIB_GLEW glew )
set( DIR_SCANNER_LIBS $ENV{PRESCANNING_LIBS_DIR} )

# glfw
include_directories( ${DIR_SCANNER_LIBS}/${LIB_GLFW}/include/ )
link_directories( ${DIR_SCANNER_LIBS}/${LIB_GLFW}/lib/ )

# glew
include_directories( ${DIR_SCANNER_LIBS}/${LIB_GLEW}/include/ )
link_directories( ${DIR_SCANNER_LIBS}/${LIB_GLEW}/lib/ )

add_executable( ${PROJECT_NAME} main_glfw.cpp )

# link
target_link_libraries( ${PROJECT_NAME} glfw3 )
target_link_libraries( ${PROJECT_NAME} glew32 )

endif()
