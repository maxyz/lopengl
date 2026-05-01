chapter_shaders()

add_executable(13_basic_light basic_light.cpp)
target_link_libraries(13_basic_light ${LIBS})

add_executable(13_ex_1 ex_1.cpp)
target_link_libraries(13_ex_1 ${LIBS})

add_executable(13_ex_2 ex_2.cpp)
target_link_libraries(13_ex_2 ${LIBS})

add_executable(13_ex_2_view ex_2_view.cpp)
target_link_libraries(13_ex_2_view ${LIBS})

add_executable(13_ex_3 ex_3.cpp)
target_link_libraries(13_ex_3 ${LIBS})
