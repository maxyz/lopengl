#pragma once

#include <GLFW/glfw3.h>

struct movement_input_t {
  bool forward{};
  bool back{};
  bool left{};
  bool right{};
  bool up{};
  bool down{};
};

struct input_t {
  bool fov_inc{};
  bool fov_dec{};
  movement_input_t camera{};
  bool cam_yaw_left{};
  bool cam_yaw_right{};
  movement_input_t light{};
};

inline void process_common_input(GLFWwindow *window, input_t &input) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    input.camera.forward = true;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    input.camera.back = true;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    input.camera.left = true;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    input.camera.right = true;
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    input.cam_yaw_left = true;
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    input.cam_yaw_right = true;
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    input.camera.down = true;
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    input.camera.up = true;
}
