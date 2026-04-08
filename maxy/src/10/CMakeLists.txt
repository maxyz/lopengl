add_executable(10_camera_circle camera_circle.cpp)
target_link_libraries(10_camera_circle ${LIBS})
add_custom_command(TARGET 10_camera_circle POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:10_camera_circle>/shaders)

add_executable(10_walk_around walk_around.cpp)
target_link_libraries(10_walk_around ${LIBS})

add_executable(10_mouse_input mouse_input.cpp)
target_link_libraries(10_mouse_input ${LIBS})

add_executable(10_camera_class camera_class.cpp)
target_link_libraries(10_camera_class ${LIBS})
