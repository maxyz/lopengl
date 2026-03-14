add_executable(textures textures.cpp)
target_link_libraries(textures ${LIBS})
add_custom_command(TARGET textures POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:textures>/shaders)
