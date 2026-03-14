file(GLOB ex_5.8.1_sources 1.cpp)
add_executable(ex_5.8.1 ${ex_5.8.1_sources})
target_link_libraries(ex_5.8.1 ${LIBS})

file(GLOB ex_5.8.2_sources 2.cpp)
add_executable(ex_5.8.2 ${ex_5.8.2_sources})
target_link_libraries(ex_5.8.2 ${LIBS})

file(GLOB ex_5.8.3_sources 3.cpp)
add_executable(ex_5.8.3 ${ex_5.8.3_sources})
target_link_libraries(ex_5.8.3 ${LIBS})
