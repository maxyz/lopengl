add_executable(17_multiple multiple.cpp)
target_link_libraries(17_multiple ${LIBS})
add_custom_command(TARGET 17_multiple POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:17_multiple>/shaders)

add_executable(17_ex_1 ex_1.cpp)
target_link_libraries(17_ex_1 ${LIBS})
add_custom_command(TARGET 17_ex_1 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:17_ex_1>/shaders)
