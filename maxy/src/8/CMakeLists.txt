add_executable(8_trying_glm trying_glm.cpp)
target_link_libraries(8_trying_glm ${LIBS})
add_custom_command(TARGET 8_trying_glm POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:8_trying_glm>/shaders)
