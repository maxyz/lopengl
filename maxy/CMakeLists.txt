cmake_minimum_required(VERSION 3.24)
project(lopengl)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
set(LIBS ${LIBS} ${OPENGL_gl_LIBRARY})

find_package(glfw3 REQUIRED)
set(LIBS ${LIBS} glfw)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

add_library(GLAD src/gl.c)
set(LIBS ${LIBS} GLAD)

file(GLOB common_sources src/common/*.cpp)
add_library(COMMON ${common_sources})
set(LIBS ${LIBS} COMMON)

file(GLOB lo01_sources src/lo01/*.cpp)
add_executable(lo01 ${lo01_sources})
target_link_libraries(lo01 ${LIBS})

file(GLOB triangle_sources src/triangle/*.cpp)
add_executable(triangle ${triangle_sources})
target_link_libraries(triangle ${LIBS})

file(GLOB triangles_sources src/triangles/*.cpp)
add_executable(triangles ${triangles_sources})
target_link_libraries(triangles ${LIBS})

file(GLOB ex_5.8.1_sources src/ex_5.8/1.cpp)
add_executable(ex_5.8.1 ${ex_5.8.1_sources})
target_link_libraries(ex_5.8.1 ${LIBS})

file(GLOB ex_5.8.2_sources src/ex_5.8/2.cpp)
add_executable(ex_5.8.2 ${ex_5.8.2_sources})
target_link_libraries(ex_5.8.2 ${LIBS})

file(GLOB ex_5.8.3_sources src/ex_5.8/3.cpp)
add_executable(ex_5.8.3 ${ex_5.8.3_sources})
target_link_libraries(ex_5.8.3 ${LIBS})

add_executable(query_attributes src/queries/main.cpp)
target_link_libraries(query_attributes ${LIBS})

add_executable(6_shader_share src/6/shader_share.cpp)
target_link_libraries(6_shader_share ${LIBS})

add_executable(6_uniforms src/6/uniforms.cpp)
target_link_libraries(6_uniforms ${LIBS})

add_executable(6_attributes src/6/attributes.cpp)
target_link_libraries(6_attributes ${LIBS})

add_executable(6_shaders_class src/6/shaders_class.cpp)
target_link_libraries(6_shaders_class ${LIBS})
add_custom_command(TARGET 6_shaders_class POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/6/shaders
    $<TARGET_FILE_DIR:6_shaders_class>/shaders)

add_subdirectory(src/ex_6.8)
