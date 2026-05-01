chapter_shaders()

add_executable(16_directional directional.cpp)
target_link_libraries(16_directional ${LIBS})

add_executable(16_positional positional.cpp)
target_link_libraries(16_positional ${LIBS})

add_executable(16_spot spot.cpp)
target_link_libraries(16_spot ${LIBS})
