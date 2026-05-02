#include <expected>
#include <glm/geometric.hpp>
#include <iostream>
#include <utility>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "common/common.hpp"

const char *TITLE = "LOpenGL";
const GLuint WIDTH = 1024;
const GLuint HEIGHT = 768;

struct view_t {
  float width = WIDTH;
  float height = HEIGHT;
};

struct state_t {
  view_t viewport = {.width = WIDTH, .height = HEIGHT};
  Camera camera = Camera(glm::vec3(0.f, 0.f, 3.f));
  light_directional_t light = {.direction = glm::vec3(-.2f, -1.f, -.3f),
                               .ambient = glm::vec3(.2f, .2f, .2f),
                               .diffuse = glm::vec3(.5f, .5f, .5f),
                               .specular = glm::vec3(1.f, 1.f, 1.f)};

  bool mouse_new_focus = true;
};
// Global state
state_t state;

struct SceneRenderer {
  struct programs_t {
    id_t view{};
  };
  struct vaos_t {
    id_t cube{};
  };
  struct textures_t {
    id_t diffuse{};
    id_t specular{};
  };

  programs_t m_ps{};
  vaos_t m_vs{};
  textures_t m_ts{};
  id_t m_vbo{};
  GLFWwindow *m_window{};

  static std::expected<SceneRenderer, std::string> create(GLFWwindow *window);
  void render(input_t input, float delta);

  ~SceneRenderer();
  SceneRenderer() = default;
  SceneRenderer(const SceneRenderer &) = delete;
  SceneRenderer &operator=(const SceneRenderer &) = delete;
  SceneRenderer(SceneRenderer &&o) noexcept
      : m_ps(std::exchange(o.m_ps, {})), m_vs(std::exchange(o.m_vs, {})),
        m_ts(std::exchange(o.m_ts, {})), m_vbo(std::exchange(o.m_vbo, 0)),
        m_window(std::exchange(o.m_window, nullptr)) {}
  SceneRenderer &operator=(SceneRenderer &&) = delete;
};

void init_window_callbacks(GLFWwindow *window);
void event_loop(GLFWwindow *window, SceneRenderer &renderer);

int main() {
  auto ctx = GLContext::create(WIDTH, HEIGHT, TITLE);
  if (!ctx) {
    std::cerr << ctx.error() << "\n";
    return -1;
  }

  init_window_callbacks(ctx->window());

  auto renderer = SceneRenderer::create(ctx->window());
  if (!renderer) {
    std::cerr << renderer.error() << "\n";
    return -1;
  }
  event_loop(ctx->window(), *renderer);
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void window_focus_callback(GLFWwindow *window, int focused);

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

void init_window_callbacks(GLFWwindow *window) {
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetWindowFocusCallback(window, window_focus_callback);

  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  state.viewport.width = width;
  state.viewport.height = height;
  glViewport(0, 0, width, height);
}

void window_focus_callback(GLFWwindow *window, int focused) {
  if (!focused) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    state.mouse_new_focus = true;
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureKeyboard) {
    return;
  }

  if ((key == GLFW_KEY_GRAVE_ACCENT) &&
      (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    auto input_mode = glfwGetInputMode(window, GLFW_CURSOR);
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      state.mouse_new_focus = true;
    }
  }
}

const float MOUSE_SENSITIVITY = .1f;

void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
  static float x = state.viewport.width / 2;
  static float y = state.viewport.height / 2;

  ImGuiIO &io = ImGui::GetIO();
  auto input_mode = glfwGetInputMode(window, GLFW_CURSOR);
  if ((input_mode == GLFW_CURSOR_NORMAL) ||
      ((input_mode == GLFW_CURSOR_DISABLED) && io.WantCaptureMouse)) {
    return;
  }

  if (state.mouse_new_focus) {
    x = x_pos;
    y = y_pos;
    state.mouse_new_focus = false;
  }

  float x_offset = x_pos - x;
  float y_offset = y - y_pos;
  x = x_pos;
  y = y_pos;
  state.camera.process_rotation(x_offset, y_offset);
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureMouse) {
    return;
  }

  state.camera.update_fov(static_cast<float>(y_offset));
}

void process_events(input_t input, float delta);

std::expected<SceneRenderer, std::string>
SceneRenderer::create(GLFWwindow *window) {
  auto shader = Shader::build("shaders/16_directional.vert",
                              "shaders/16_directional.frag");
  if (!shader) {
    return std::unexpected(shader.error());
  }

  auto load_texture_res = load_texture("textures/container2.png");
  if (!load_texture_res) {
    return std::unexpected(load_texture_res.error());
  }
  auto load_texture_specular_res =
      load_texture("textures/container2_specular.png");
  if (!load_texture_specular_res) {
    return std::unexpected(load_texture_specular_res.error());
  }

  unsigned int cube_vao;
  unsigned int vbo;
  glGenVertexArrays(1, &cube_vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(cube_vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t),
      reinterpret_cast<void *>(offsetof(cube_vertex_t, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t),
      reinterpret_cast<void *>(offsetof(cube_vertex_t, normal)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t),
      reinterpret_cast<void *>(offsetof(cube_vertex_t, texcoord)));
  glEnableVertexAttribArray(2);

  SceneRenderer r;
  r.m_ps = {.view = shader->ID};
  r.m_vs = {.cube = cube_vao};
  r.m_ts = {
      .diffuse = *load_texture_res,
      .specular = *load_texture_specular_res,
  };
  r.m_vbo = vbo;
  r.m_window = window;

  shader->use();
  shader->set_int("material.diffuse", 0);

  return r;
}

void SceneRenderer::render(input_t input, float delta) {
  auto now = glfwGetTime();
  process_events(input, delta);

  glUseProgram(m_ps.view);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_ts.diffuse);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_ts.specular);
  specular_map_t specular_map = {
      .diffuse = 0,
      .specular = 1,
      .shininess = 64.f,
  };

  glm::mat4 model;

  glm::mat4 view = state.camera.get_view_matrix();

  glm::mat4 projection =
      glm::perspective(glm::radians(state.camera.fov),
                       static_cast<float>(state.viewport.width) /
                           static_cast<float>(state.viewport.height),
                       .1f, 100.f);

  set_mat4(m_ps.view, "view", view);
  set_mat4(m_ps.view, "projection", projection);
  set_vec3(m_ps.view, "view_pos", state.camera.position);
  set_directional_light(m_ps.view, "light", state.light);
  set_specular_map(m_ps.view, "material", specular_map);

  glBindVertexArray(m_vs.cube);

  float angle;
  for (unsigned int i = 0; i < 10; ++i) {
    angle = 20.f * i;
    angle = glfwGetTime() * (i % 3) * 25.f;
    model = glm::translate(glm::mat4(1.f), example_cube_positions[i]);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));

    glUseProgram(m_ps.view);
    set_mat4(m_ps.view, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiIO &io = ImGui::GetIO();
  if (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  } else {
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
  }

  ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
  ImGui::Begin("Scene information", NULL, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::PushItemWidth(150.0f);

  ImGui::LabelText("Pos", "(%.2f, %.2f, %.2f)", state.camera.position.x,
                   state.camera.position.y, state.camera.position.z);
  double x, y;
  glfwGetCursorPos(m_window, &x, &y);
  ImGui::LabelText("Mouse", "(%.2f, %.2f)", x, y);
  if (glfwGetInputMode(m_window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
    ImGui::SeparatorText("Light");
    ImGui::DragFloat3("Direction", glm::value_ptr(state.light.direction), .01f,
                      -1.f, 1.f);
    ImGui::ColorEdit3("Ambience", glm::value_ptr(state.light.ambient));
    ImGui::ColorEdit3("Diffuse", glm::value_ptr(state.light.diffuse));
    ImGui::ColorEdit3("Specular", glm::value_ptr(state.light.specular));
  }
  ImGui::PopItemWidth();

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

SceneRenderer::~SceneRenderer() {
  if (!m_vbo)
    return;
  glDeleteVertexArrays(1, &m_vs.cube);
  glDeleteBuffers(1, &m_vbo);
}

void process_events(input_t input, float delta) {
  if (input.fov_inc) {
    state.camera.update_fov(1.f);
  }
  if (input.fov_dec) {
    state.camera.update_fov(-1.f);
  }
  if (input.cam_up) {
    state.camera.process_movement(CameraMovement::UP, delta);
  }
  if (input.cam_down) {
    state.camera.process_movement(CameraMovement::DOWN, delta);
  }
  if (input.cam_left) {
    state.camera.process_movement(CameraMovement::LEFT, delta);
  }
  if (input.cam_right) {
    state.camera.process_movement(CameraMovement::RIGHT, delta);
  }
  if (input.cam_forward) {
    state.camera.process_movement(CameraMovement::FORWARD, delta);
  }
  if (input.cam_back) {
    state.camera.process_movement(CameraMovement::BACKWARD, delta);
  }
  if (input.cam_yaw_left) {
    state.camera.process_rotation(120 * -SPEED * delta, 0.f);
  }
  if (input.cam_yaw_right) {
    state.camera.process_rotation(120 * SPEED * delta, 0.f);
  }
}

void process_input(GLFWwindow *window, input_t &input);

struct delta_t {
  float last;  // Time of last frame
  float delta; // Time between current frame and last frame
};

void update_delta(delta_t &delta) {
  float now = glfwGetTime();
  delta.delta = now - delta.last;
  delta.last = now;
}

void event_loop(GLFWwindow *window, SceneRenderer &renderer) {
  input_t input{};

  delta_t delta{};

  while (!glfwWindowShouldClose(window)) {
    update_delta(delta);

    input = {};
    process_input(window, input);
    glClearColor(.2f, .3f, .3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer.render(input, delta.delta);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void process_input(GLFWwindow *window, input_t &input) {
  process_common_input(window, input);
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    input.light_forward = true;
  }
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    input.light_back = true;
  }
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    input.light_left = true;
  }
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    input.light_right = true;
  }
  if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
    input.light_up = true;
  }
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
    input.light_down = true;
  }
}
