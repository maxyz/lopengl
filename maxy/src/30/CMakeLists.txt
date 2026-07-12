chapter_shaders()

add_executable(30_houses houses.cpp)
target_link_libraries(30_houses ${LIBS})

add_executable(30_exploding exploding.cpp)
target_link_libraries(30_exploding ${LIBS})

add_executable(30_normals normals.cpp)
target_link_libraries(30_normals ${LIBS})
