#pragma once

#include <glad/gl.h>

#include "common/input.hpp"
#include "common/window_state.hpp"

struct frame_time_t {
  float last_time{};
  float delta{};
};

inline void update_delta(frame_time_t &d) {
  float now = static_cast<float>(glfwGetTime());
  d.delta = now - d.last_time;
  d.last_time = now;
}

inline void process_camera_events(window_state_t &ws, input_t input,
                                  float delta) {
  if (input.fov_inc)
    ws.camera.update_fov(1.f);
  if (input.fov_dec)
    ws.camera.update_fov(-1.f);
  if (input.cam_up)
    ws.camera.process_movement(CameraMovement::UP, delta);
  if (input.cam_down)
    ws.camera.process_movement(CameraMovement::DOWN, delta);
  if (input.cam_left)
    ws.camera.process_movement(CameraMovement::LEFT, delta);
  if (input.cam_right)
    ws.camera.process_movement(CameraMovement::RIGHT, delta);
  if (input.cam_forward)
    ws.camera.process_movement(CameraMovement::FORWARD, delta);
  if (input.cam_back)
    ws.camera.process_movement(CameraMovement::BACKWARD, delta);
  if (input.cam_yaw_left)
    ws.camera.process_rotation(120 * -camera_speed * delta, 0.f);
  if (input.cam_yaw_right)
    ws.camera.process_rotation(120 * camera_speed * delta, 0.f);
}

template <typename Renderer, typename InputFn>
void event_loop(GLFWwindow *window, Renderer &renderer, InputFn process_input) {
  input_t input{};
  frame_time_t delta{};

  while (!glfwWindowShouldClose(window)) {
    update_delta(delta);

    input = {};
    process_input(window, input);

    renderer.render(input, delta.delta);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}
