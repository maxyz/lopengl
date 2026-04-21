add_executable(14_materials materials.cpp)
target_link_libraries(14_materials ${LIBS})
add_custom_command(TARGET 14_materials POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:14_materials>/shaders)

add_executable(14_lighting lighting.cpp)
target_link_libraries(14_lighting ${LIBS})
add_custom_command(TARGET 14_lighting POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:14_lighting>/shaders)

add_executable(14_strobe strobe.cpp)
target_link_libraries(14_strobe ${LIBS})
add_custom_command(TARGET 14_strobe POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:14_strobe>/shaders)

add_executable(14_ex_1 ex_1.cpp)
target_link_libraries(14_ex_1 ${LIBS})
add_custom_command(TARGET 14_ex_1 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:14_ex_1>/shaders)

add_executable(14_ex_2 ex_2.cpp)
target_link_libraries(14_ex_2 ${LIBS})
add_custom_command(TARGET 14_ex_2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:14_ex_2>/shaders)
