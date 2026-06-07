add_executable(sdl3_21_model model.cpp)
target_link_libraries(sdl3_21_model sdl3_engine assimp::assimp)
chapter_spv_shaders(sdl3_21_model)
