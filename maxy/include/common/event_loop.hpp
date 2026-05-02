#pragma once

#include <glad/gl.h>

#include "common/input.hpp"

struct delta_t {
  float last{};
  float delta{};
};

inline void update_delta(delta_t &d) {
  float now = static_cast<float>(glfwGetTime());
  d.delta = now - d.last;
  d.last = now;
}

template <typename Renderer, typename InputFn>
void event_loop(GLFWwindow *window, Renderer &renderer, InputFn process_input) {
  input_t input{};
  delta_t delta{};

  while (!glfwWindowShouldClose(window)) {
    update_delta(delta);

    input = {};
    process_input(window, input);

    renderer.render(input, delta.delta);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}
