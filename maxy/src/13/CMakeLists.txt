add_executable(13_basic_light basic_light.cpp)
target_link_libraries(13_basic_light ${LIBS})
add_custom_command(TARGET 13_basic_light POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:13_basic_light>/shaders)

add_executable(13_ex_1 ex_1.cpp)
target_link_libraries(13_ex_1 ${LIBS})
add_custom_command(TARGET 13_ex_1 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:13_ex_1>/shaders)

add_executable(13_ex_2 ex_2.cpp)
target_link_libraries(13_ex_2 ${LIBS})
add_custom_command(TARGET 13_ex_2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:13_ex_2>/shaders)

add_executable(13_ex_2_view ex_2_view.cpp)
target_link_libraries(13_ex_2_view ${LIBS})
add_custom_command(TARGET 13_ex_2_view POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:13_ex_2_view>/shaders)

add_executable(13_ex_3 ex_3.cpp)
target_link_libraries(13_ex_3 ${LIBS})
add_custom_command(TARGET 13_ex_3 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:13_ex_3>/shaders)

