chapter_shaders()

add_executable(15_diffuse_map diffuse_map.cpp)
target_link_libraries(15_diffuse_map ${LIBS})

add_executable(15_specular_map specular_map.cpp)
target_link_libraries(15_specular_map ${LIBS})

add_executable(15_light_controls light_controls.cpp)
target_link_libraries(15_light_controls ${LIBS})

add_executable(15_ex_2 ex_2.cpp)
target_link_libraries(15_ex_2 ${LIBS})

add_executable(15_emission_map emission_map.cpp)
target_link_libraries(15_emission_map ${LIBS})
