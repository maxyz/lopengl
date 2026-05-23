chapter_shaders()

add_executable(24_grass grass.cpp)
target_link_libraries(24_grass ${LIBS})

add_executable(24_windows windows.cpp)
target_link_libraries(24_windows ${LIBS})

add_executable(24_multi_light_windows multi_light_windows.cpp)
target_link_libraries(24_multi_light_windows ${LIBS})
