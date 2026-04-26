add_executable(16_directional directional.cpp)
target_link_libraries(16_directional ${LIBS})
add_custom_command(TARGET 16_directional POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:16_directional>/shaders)

add_executable(16_positional positional.cpp)
target_link_libraries(16_positional ${LIBS})
add_custom_command(TARGET 16_positional POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:16_positional>/shaders)

add_executable(16_spot spot.cpp)
target_link_libraries(16_spot ${LIBS})
add_custom_command(TARGET 16_spot POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:16_spot>/shaders)
