cmake_minimum_required(VERSION 3.24)
project(lopengl)

option(DEVELOPMENT_MODE "Enable source-tree resource lookup" ON)
if (DEVELOPMENT_MODE)
  add_compile_definitions(ASSETS_PATH="${CMAKE_SOURCE_DIR}/assets/")
  # Enable address sanitization
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
else()
  add_compile_definitions(
    ASSETS_PATH="${CMAKE_INSTALL_PREFIX}/share/lopengl/assets/"
  )
endif()

add_compile_options(-Wall)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
set(LIBS ${LIBS} ${OPENGL_gl_LIBRARY})
include_directories(${OPENGL_INCLUDE_DIR})

find_package(glfw3 REQUIRED)
set(LIBS ${LIBS} glfw)

find_package(glm REQUIRED)

find_package(assimp REQUIRED)
set(LIBS ${LIBS} ${ASSIMP_LIBRARIES})
include_directories(${ASSIMP_INCLUDE_DIRS})

find_package(SDL3)
if(SDL3_FOUND)
  message(STATUS "SDL3 ${SDL3_VERSION} found -- SDL3 examples will be built")
  find_package(SDL3_image REQUIRED)
  message(STATUS "SDL3_image ${SDL3_image_VERSION} found")
else()
  message(STATUS "SDL3 not found -- SDL3 examples will be skipped")
endif()

find_package(PkgConfig REQUIRED)

pkg_check_modules(STB REQUIRED IMPORTED_TARGET stb)
set(LIBS ${LIBS} PkgConfig::STB)
include_directories(${STB_INCLUDE_DIRS})

pkg_check_modules(IMGUI REQUIRED IMPORTED_TARGET imgui)
set(LIBS ${LIBS} PkgConfig::IMGUI)
include_directories(${IMGUI_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)

# Put all executables in the root of the build folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Call once per chapter CMakeLists.txt to copy each shader file to the build
# directory when (and only when) it changes, independent of which executable
# triggered the build.
function(chapter_shaders)
  file(GLOB _sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.glsl"
  )
  get_filename_component(_chapter "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
  string(REPLACE "." "_" _chapter_safe "${_chapter}")
  set(_outputs "")
  foreach(_src ${_sources})
    get_filename_component(_name "${_src}" NAME)
    set(_dst "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${_name}")
    add_custom_command(
      OUTPUT "${_dst}"
      COMMAND ${CMAKE_COMMAND} -E copy "${_src}" "${_dst}"
      DEPENDS "${_src}"
      COMMENT "Shader: ${_name}"
    )
    list(APPEND _outputs "${_dst}")
  endforeach()
  add_custom_target(shaders_${_chapter_safe} ALL DEPENDS ${_outputs})
endfunction()

function(chapter_spv_shaders target)
  get_filename_component(_chapter "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
  string(REPLACE "." "_" _chapter_safe "${_chapter}")
  if(NOT TARGET spv_shaders_${_chapter_safe})
    file(GLOB _sources CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert"
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag"
    )
    set(_outputs "")
    foreach(_src ${_sources})
      get_filename_component(_name "${_src}" NAME)
      set(_spv "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${_chapter_safe}/${_name}.spv")
      add_custom_command(
        OUTPUT "${_spv}"
        COMMAND glslc "${_src}" -o "${_spv}"
        DEPENDS "${_src}"
        COMMENT "Compiling SPIR-V: ${_name}"
      )
      list(APPEND _outputs "${_spv}")
    endforeach()
    add_custom_target(spv_shaders_${_chapter_safe} ALL DEPENDS ${_outputs})
  endif()
  add_dependencies(${target} spv_shaders_${_chapter_safe})
endfunction()

add_subdirectory(src)
