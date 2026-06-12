chapter_shaders()

add_executable(27_skybox skybox.cpp)
target_link_libraries(27_skybox ${LIBS})

add_executable(27_environment environment.cpp)
target_link_libraries(27_environment ${LIBS})
