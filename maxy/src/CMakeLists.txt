add_library(GLAD gl.c)
set(LIBS ${LIBS} GLAD)

file(GLOB common_sources common/*.cpp)
add_library(COMMON ${common_sources})
set(LIBS ${LIBS} COMMON)

add_executable(query_attributes queries/main.cpp)
target_link_libraries(query_attributes ${LIBS})

add_subdirectory(4)

add_subdirectory(5)
add_subdirectory(ex_5.8)

add_subdirectory(6)
add_subdirectory(ex_6.8)

add_subdirectory(7)
add_subdirectory(ex_7.8)

add_subdirectory(8)
add_subdirectory(ex_8.19)
