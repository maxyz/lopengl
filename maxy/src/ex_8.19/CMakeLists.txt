add_executable(ex_8.19.1 1.cpp)
target_link_libraries(ex_8.19.1 ${LIBS})
add_custom_command(TARGET ex_8.19.1 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:ex_8.19.1>/shaders)

add_executable(ex_8.19.2 2.cpp)
target_link_libraries(ex_8.19.2 ${LIBS})
add_custom_command(TARGET ex_8.19.2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:ex_8.19.2>/shaders)
