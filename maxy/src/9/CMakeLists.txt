chapter_shaders()

add_executable(9_going_3d going_3d.cpp)
target_link_libraries(9_going_3d ${LIBS})

add_executable(9_more_3d more_3d.cpp)
target_link_libraries(9_more_3d ${LIBS})

add_executable(9_more_cubes more_cubes.cpp)
target_link_libraries(9_more_cubes ${LIBS})
