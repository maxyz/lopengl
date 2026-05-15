chapter_shaders()

add_executable(22_depth depth.cpp)
target_link_libraries(22_depth ${LIBS})

add_executable(22_linear linear.cpp)
target_link_libraries(22_linear ${LIBS})
