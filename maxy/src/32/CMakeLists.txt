chapter_shaders()

add_executable(32_msaa msaa.cpp)
target_link_libraries(32_msaa ${LIBS})

add_executable(32_offscreen offscreen.cpp)
target_link_libraries(32_offscreen ${LIBS})
