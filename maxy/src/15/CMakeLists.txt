add_executable(15_diffuse_map diffuse_map.cpp)
target_link_libraries(15_diffuse_map ${LIBS})
add_custom_command(TARGET 15_diffuse_map POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:15_diffuse_map>/shaders)

add_executable(15_specular_map specular_map.cpp)
target_link_libraries(15_specular_map ${LIBS})
add_custom_command(TARGET 15_specular_map POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:15_specular_map>/shaders)
