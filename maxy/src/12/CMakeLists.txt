add_executable(12_colors colors.cpp)
target_link_libraries(12_colors ${LIBS})
add_custom_command(TARGET 12_colors POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:12_colors>/shaders)
