cmake_minimum_required(VERSION 3.13.0)
project(geometry VERSION 0.1.0)

# linux:
if (UNIX)
    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)

    include_directories(${SDL2_INCLUDE_DIRS})
    include_directories(${OPENGL_INCLUDE_DIRS})

    add_executable(geometry main_geom.cpp)

    target_link_libraries(geometry ${SDL2_LIBRARIES})
    target_link_libraries(geometry ${OPENGL_LIBRARIES})
    target_link_libraries(geometry ${GLEW_LIBRARIES})
endif()

# windows:
if (WIN32)
    link_directories("C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x64")

    include_directories(C:/Users/garde/lib/glew-2.1.0/include/)
    link_directories(C:/Users/garde/lib/glew-2.1.0/lib/Release/x64/)

    link_directories(C:/Users/garde/lib/SDL2-2.28.4/lib/x64)
    include_directories(C:/Users/garde/lib/SDL2-2.28.4/include/)

    add_executable(geometry main_geom.cpp)

    target_link_libraries(geometry SDL2)
    target_link_libraries(geometry OpenGL32)
    target_link_libraries(geometry glew32)
    target_link_libraries(geometry glew32s)
endif()
