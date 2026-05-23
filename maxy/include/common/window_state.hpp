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

inline void
framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    auto *window_state =
        static_cast<window_state_t *>(glfwGetWindowUserPointer(window));
    window_state->viewport.width = static_cast<float>(width);
    window_state->viewport.height = static_cast<float>(height);
    glViewport(0, 0, width, height);
}

inline void window_focus_callback(GLFWwindow *window, int focused) {
    ImGui_ImplGlfw_WindowFocusCallback(window, focused);
    auto *window_state =
        static_cast<window_state_t *>(glfwGetWindowUserPointer(window));
    if (!focused) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        window_state->mouse_new_focus = true;
    }
}

inline void
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    auto *window_state =
        static_cast<window_state_t *>(glfwGetWindowUserPointer(window));
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
        return;

    if ((key == GLFW_KEY_GRAVE_ACCENT) && (action == GLFW_PRESS)) {
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_NORMAL) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            window_state->mouse_new_focus = true;
        }
    }
}

inline void
mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    auto input_mode = glfwGetInputMode(window, GLFW_CURSOR);
    if (input_mode == GLFW_CURSOR_NORMAL) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        return;
    }
}

inline void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
    auto *window_state =
        static_cast<window_state_t *>(glfwGetWindowUserPointer(window));

    auto input_mode = glfwGetInputMode(window, GLFW_CURSOR);
    if (input_mode == GLFW_CURSOR_NORMAL) {
        ImGui_ImplGlfw_CursorPosCallback(window, x_pos, y_pos);
        return;
    }

    if (window_state->mouse_new_focus) {
        window_state->last_mouse_x = static_cast<float>(x_pos);
        window_state->last_mouse_y = static_cast<float>(y_pos);
        window_state->mouse_new_focus = false;
    }

    float x_offset = static_cast<float>(x_pos) - window_state->last_mouse_x;
    float y_offset = window_state->last_mouse_y - static_cast<float>(y_pos);
    window_state->last_mouse_x = static_cast<float>(x_pos);
    window_state->last_mouse_y = static_cast<float>(y_pos);
    window_state->camera.process_rotation(x_offset, y_offset);
}

inline void
scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
    ImGui_ImplGlfw_ScrollCallback(window, x_offset, y_offset);
    auto *window_state =
        static_cast<window_state_t *>(glfwGetWindowUserPointer(window));
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;

    window_state->camera.update_fov(static_cast<float>(y_offset));
}

struct window_callbacks_t {
    GLFWframebuffersizefun framebuffer_size;
    GLFWwindowfocusfun window_focus;
    GLFWkeyfun key;
    GLFWmousebuttonfun mouse_button;
    GLFWcursorposfun cursor_pos;
    GLFWscrollfun scroll;
};

const window_callbacks_t DEFAULT_WINDOW_CALLBACKS = {
    .framebuffer_size = framebuffer_size_callback,
    .window_focus = window_focus_callback,
    .key = key_callback,
    .mouse_button = mouse_button_callback,
    .cursor_pos = mouse_callback,
    .scroll = scroll_callback,
};

inline void init_window_callbacks(
    GLFWwindow *window, window_state_t &window_state,
    window_callbacks_t callbacks = DEFAULT_WINDOW_CALLBACKS
) {
    glfwSetWindowUserPointer(window, &window_state);
    glfwSetFramebufferSizeCallback(window, callbacks.framebuffer_size);
    glfwSetWindowFocusCallback(window, callbacks.window_focus);
    glfwSetKeyCallback(window, callbacks.key);
    glfwSetMouseButtonCallback(window, callbacks.mouse_button);
    glfwSetCursorPosCallback(window, callbacks.cursor_pos);
    glfwSetScrollCallback(window, callbacks.scroll);
}
