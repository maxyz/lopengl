cmake_minimum_required(VERSION 3.24)
project(lopengl)

option(DEVELOPMENT_MODE "Enable source-tree resource lookup" ON)
if (DEVELOPMENT_MODE)
  add_compile_definitions(ASSETS_PATH="${CMAKE_SOURCE_DIR}/assets/")
else()
  add_compile_definitions(
    ASSETS_PATH="${CMAKE_INSTALL_PREFIX}/share/lopengl/assets/"
  )
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
set(LIBS ${LIBS} ${OPENGL_gl_LIBRARY})

find_package(glfw3 REQUIRED)
set(LIBS ${LIBS} glfw)

find_package(glm REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(STB REQUIRED IMPORTED_TARGET stb)
set(LIBS ${LIBS} PkgConfig::STB)

include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Put all executables in the root of the build folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)
