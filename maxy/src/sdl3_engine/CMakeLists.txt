# The system libimgui ships only OpenGL/SDL2 backends pre-compiled.
# The SDL3 GPU backends must be built from the dev package example sources.
pkg_get_variable(IMGUI_PREFIX imgui prefix)
pkg_get_variable(IMGUI_INCLUDEDIR imgui includedir)
set(IMGUI_BACKENDS_SRC ${IMGUI_PREFIX}/share/doc/libimgui-dev/examples/backends)

add_library(sdl3_engine
    engine.cpp
    ${IMGUI_BACKENDS_SRC}/imgui_impl_sdl3.cpp
    ${IMGUI_BACKENDS_SRC}/imgui_impl_sdlgpu3.cpp
)
target_include_directories(sdl3_engine
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${IMGUI_INCLUDE_DIRS}
    PRIVATE ${IMGUI_INCLUDEDIR}/backends
)
target_link_libraries(sdl3_engine PUBLIC SDL3::SDL3 SDL3_image::SDL3_image glm::glm PkgConfig::IMGUI)
