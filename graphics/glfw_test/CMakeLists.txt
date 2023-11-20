cmake_minimum_required(VERSION 3.13.0)
project(glfw_test VERSION 0.1.0)

# Linux:
if (UNIX)
message(FATAL_EFFOR "UNIX configuration")
# TODO: impl.

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
target_link_libraries( ${PROJECT_NAME} glew32s )

endif()
