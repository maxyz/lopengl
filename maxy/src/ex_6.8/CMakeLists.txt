add_executable(ex_6.8.1 1.cpp)
target_link_libraries(ex_6.8.1 ${LIBS})
add_custom_command(TARGET ex_6.8.1 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:ex_6.8.1>/shaders)

add_executable(ex_6.8.2 2.cpp)
target_link_libraries(ex_6.8.2 ${LIBS})
add_custom_command(TARGET ex_6.8.2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:ex_6.8.2>/shaders)

add_executable(ex_6.8.3 3.cpp)
target_link_libraries(ex_6.8.3 ${LIBS})
add_custom_command(TARGET ex_6.8.3 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:ex_6.8.3>/shaders)

