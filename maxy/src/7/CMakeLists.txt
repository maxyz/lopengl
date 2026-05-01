chapter_shaders()

add_executable(7_texture texture.cpp)
target_link_libraries(7_texture ${LIBS})

add_executable(7_texture_disco texture_disco.cpp)
target_link_libraries(7_texture_disco ${LIBS})

add_executable(7_combined combined.cpp)
target_link_libraries(7_combined ${LIBS})
