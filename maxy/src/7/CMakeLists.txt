add_executable(7_texture texture.cpp)
target_link_libraries(7_texture ${LIBS})
add_custom_command(TARGET 7_texture POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:7_texture>/shaders)

add_executable(7_texture_disco texture_disco.cpp)
target_link_libraries(7_texture_disco ${LIBS})
add_custom_command(TARGET 7_texture_disco POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:7_texture_disco>/shaders)

add_executable(7_combined combined.cpp)
target_link_libraries(7_combined ${LIBS})
add_custom_command(TARGET 7_combined POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:7_combined>/shaders)
