add_executable(9_going_3d going_3d.cpp)
target_link_libraries(9_going_3d ${LIBS})
add_custom_command(TARGET 9_going_3d POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:9_going_3d>/shaders)

add_executable(9_more_3d more_3d.cpp)
target_link_libraries(9_more_3d ${LIBS})

add_executable(9_more_cubes more_cubes.cpp)
target_link_libraries(9_more_cubes ${LIBS})
