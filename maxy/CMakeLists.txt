cmake_minimum_required(VERSION 3.24)
project(lopengl)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(GLAD src/gl.c)
set(LIBS ${LIBS} GLAD)

file(GLOB lo01_sources src/lo01/*.cpp)
add_executable(lo01 ${lo01_sources})
target_link_libraries(lo01 ${OPENGL_gl_LIBRARY} glfw ${LIBS})

file(GLOB triangle_sources src/triangle/*.cpp)
add_executable(triangle ${triangle_sources})
target_link_libraries(triangle ${OPENGL_gl_LIBRARY} glfw ${LIBS})

file(GLOB triangles_sources src/triangles/*.cpp)
add_executable(triangles ${triangles_sources})
target_link_libraries(triangles ${OPENGL_gl_LIBRARY} glfw ${LIBS})

file(GLOB ex_5.8.1_sources src/ex_5.8/1.cpp)
add_executable(ex_5.8.1 ${ex_5.8.1_sources})
target_link_libraries(ex_5.8.1 ${OPENGL_gl_LIBRARY} glfw ${LIBS})

file(GLOB ex_5.8.2_sources src/ex_5.8/2.cpp)
add_executable(ex_5.8.2 ${ex_5.8.2_sources})
target_link_libraries(ex_5.8.2 ${OPENGL_gl_LIBRARY} glfw ${LIBS})

file(GLOB ex_5.8.3_sources src/ex_5.8/3.cpp)
add_executable(ex_5.8.3 ${ex_5.8.3_sources})
target_link_libraries(ex_5.8.3 ${OPENGL_gl_LIBRARY} glfw ${LIBS})
