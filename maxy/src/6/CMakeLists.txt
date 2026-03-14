add_executable(6_shader_share shader_share.cpp)
target_link_libraries(6_shader_share ${LIBS})

add_executable(6_uniforms uniforms.cpp)
target_link_libraries(6_uniforms ${LIBS})

add_executable(6_attributes attributes.cpp)
target_link_libraries(6_attributes ${LIBS})

add_executable(6_shaders_class shaders_class.cpp)
target_link_libraries(6_shaders_class ${LIBS})
add_custom_command(TARGET 6_shaders_class POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/6/shaders
    $<TARGET_FILE_DIR:6_shaders_class>/shaders)
