#pragma once

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <backends/imgui_impl_glfw.h>
#include <imgui.h>

#include "common/camera.hpp"

struct viewport_t {
  float width{};
  float height{};
};

struct window_state_t {
  viewport_t viewport{};
  Camera camera{glm::vec3(0.f, 0.f, 3.f)};
  bool mouse_new_focus{true};
  float last_mouse_x{};
  float last_mouse_y{};
};

inline void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  auto *ws = static_cast<window_state_t *>(glfwGetWindowUserPointer(window));
  ws->viewport.width = static_cast<float>(width);
  ws->viewport.height = static_cast<float>(height);
  glViewport(0, 0, width, height);
}

inline void window_focus_callback(GLFWwindow *window, int focused) {
  auto *ws = static_cast<window_state_t *>(glfwGetWindowUserPointer(window));
  if (!focused) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    ws->mouse_new_focus = true;
  }
}

inline void key_callback(GLFWwindow *window, int key, int /*scancode*/,
                         int action, int /*mods*/) {
  auto *ws = static_cast<window_state_t *>(glfwGetWindowUserPointer(window));
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureKeyboard)
    return;

  if ((key == GLFW_KEY_GRAVE_ACCENT) &&
      (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      ws->mouse_new_focus = true;
    }
  }
}

inline void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
  auto *ws = static_cast<window_state_t *>(glfwGetWindowUserPointer(window));

  ImGuiIO &io = ImGui::GetIO();
  auto input_mode = glfwGetInputMode(window, GLFW_CURSOR);
  if ((input_mode == GLFW_CURSOR_NORMAL) ||
      ((input_mode == GLFW_CURSOR_DISABLED) && io.WantCaptureMouse)) {
    return;
  }

  if (ws->mouse_new_focus) {
    ws->last_mouse_x = static_cast<float>(x_pos);
    ws->last_mouse_y = static_cast<float>(y_pos);
    ws->mouse_new_focus = false;
  }

  float x_offset = static_cast<float>(x_pos) - ws->last_mouse_x;
  float y_offset = ws->last_mouse_y - static_cast<float>(y_pos);
  ws->last_mouse_x = static_cast<float>(x_pos);
  ws->last_mouse_y = static_cast<float>(y_pos);
  ws->camera.process_rotation(x_offset, y_offset);
}

inline void scroll_callback(GLFWwindow *window, double /*x_offset*/,
                            double y_offset) {
  auto *ws = static_cast<window_state_t *>(glfwGetWindowUserPointer(window));
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureMouse)
    return;

  ws->camera.update_fov(static_cast<float>(y_offset));
}

inline void init_window_callbacks(GLFWwindow *window, window_state_t &ws) {
  glfwSetWindowUserPointer(window, &ws);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetWindowFocusCallback(window, window_focus_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
}
