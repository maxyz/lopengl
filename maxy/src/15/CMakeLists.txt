add_executable(15_diffuse_map diffuse_map.cpp)
target_link_libraries(15_diffuse_map ${LIBS})
add_custom_command(TARGET 15_diffuse_map POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:15_diffuse_map>/shaders)

add_executable(15_specular_map specular_map.cpp)
target_link_libraries(15_specular_map ${LIBS})
add_custom_command(TARGET 15_specular_map POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:15_specular_map>/shaders)

add_executable(15_light_controls light_controls.cpp)
target_link_libraries(15_light_controls ${LIBS})
add_custom_command(TARGET 15_light_controls POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:15_light_controls>/shaders)

add_executable(15_ex_2 ex_2.cpp)
target_link_libraries(15_ex_2 ${LIBS})
add_custom_command(TARGET 15_ex_2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:15_ex_2>/shaders)

add_executable(15_emission_map emission_map.cpp)
target_link_libraries(15_emission_map ${LIBS})
add_custom_command(TARGET 15_emission_map POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:15_emission_map>/shaders)
