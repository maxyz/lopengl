add_library(sdl3_engine engine.cpp)
target_include_directories(sdl3_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(sdl3_engine PUBLIC SDL3::SDL3 glm::glm)
