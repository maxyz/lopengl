#pragma once

#include <GLFW/glfw3.h>

struct input_t {
  bool fov_inc{};
  bool fov_dec{};
  bool cam_up{};
  bool cam_down{};
  bool cam_left{};
  bool cam_right{};
  bool cam_forward{};
  bool cam_back{};
  bool cam_yaw_left{};
  bool cam_yaw_right{};
  bool light_up{};
  bool light_down{};
  bool light_left{};
  bool light_right{};
  bool light_forward{};
  bool light_back{};
};

inline void process_common_input(GLFWwindow *window, input_t &input) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    input.cam_forward = true;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    input.cam_back = true;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    input.cam_left = true;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    input.cam_right = true;
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    input.cam_yaw_left = true;
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    input.cam_yaw_right = true;
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    input.cam_down = true;
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    input.cam_up = true;
}
