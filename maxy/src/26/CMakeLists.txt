chapter_shaders()

add_executable(26_framebuffer framebuffer.cpp)
target_link_libraries(26_framebuffer ${LIBS})

add_executable(26_framebuffer_mirror framebuffer_mirror.cpp)
target_link_libraries(26_framebuffer_mirror ${LIBS})
