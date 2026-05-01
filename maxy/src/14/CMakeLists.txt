chapter_shaders()

add_executable(14_materials materials.cpp)
target_link_libraries(14_materials ${LIBS})

add_executable(14_lighting lighting.cpp)
target_link_libraries(14_lighting ${LIBS})

add_executable(14_strobe strobe.cpp)
target_link_libraries(14_strobe ${LIBS})

add_executable(14_ex_1 ex_1.cpp)
target_link_libraries(14_ex_1 ${LIBS})

add_executable(14_ex_2 ex_2.cpp)
target_link_libraries(14_ex_2 ${LIBS})

add_executable(14_imgui imgui.cpp)
target_link_libraries(14_imgui ${LIBS})
