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
  light_t light = {.position = glm::vec3(2.f, 1.f, -2.f),
                   .ambient = glm::vec3(.2f, .2f, .2f),
                   .diffuse = glm::vec3(.5f, .5f, .5f),
                   .specular = glm::vec3(1.f, 1.f, 1.f)};

  bool mouse_new_focus = true;
};
// Global state
state_t state;

struct SceneRenderer {
  id_t p{};
  id_t light_id{};
  id_t cube_vao{};
  id_t light_vao{};
  id_t texture{};
  id_t texture_specular{};
  id_t m_vbo{};
  GLFWwindow *m_window{};

  static std::expected<SceneRenderer, std::string> create(GLFWwindow *window);
  void render(input_t input, float delta);

  ~SceneRenderer();
  SceneRenderer() = default;
  SceneRenderer(const SceneRenderer &) = delete;
  SceneRenderer &operator=(const SceneRenderer &) = delete;
  SceneRenderer(SceneRenderer &&o) noexcept
      : p(std::exchange(o.p, 0)), light_id(std::exchange(o.light_id, 0)),
        cube_vao(std::exchange(o.cube_vao, 0)),
        light_vao(std::exchange(o.light_vao, 0)),
        texture(std::exchange(o.texture, 0)),
        texture_specular(std::exchange(o.texture_specular, 0)),
        m_vbo(std::exchange(o.m_vbo, 0)),
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

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void window_focus_callback(GLFWwindow *window, int focused);

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
    glfwSetCursorPosCallback(window, NULL);

  } else {
    state.mouse_new_focus = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
  }
}

void process_events(input_t input, float delta);

std::expected<SceneRenderer, std::string>
SceneRenderer::create(GLFWwindow *window) {
  auto shader = Shader::build("shaders/15_view_specular.vert",
                              "shaders/15_view_specular.frag");
  if (!shader) {
    return std::unexpected(shader.error());
  }
  auto light_shader =
      Shader::build("shaders/15_light.vert", "shaders/15_light.frag");
  if (!light_shader) {
    return std::unexpected(light_shader.error());
  }
  auto load_texture_res = load_texture("textures/container2.png");
  if (!load_texture_res) {
    return std::unexpected(load_texture_res.error());
  }
  auto load_texture_specular_res =
      load_texture("textures/container2_specular.png");
  if (!load_texture_res) {
    return std::unexpected(load_texture_res.error());
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

  unsigned int light_vao;
  glGenVertexArrays(1, &light_vao);
  glBindVertexArray(light_vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t),
      reinterpret_cast<void *>(offsetof(cube_vertex_t, position)));
  glEnableVertexAttribArray(0);

  SceneRenderer r;
  r.p = shader->ID;
  r.light_id = light_shader->ID;
  r.texture = *load_texture_res;
  r.texture_specular = *load_texture_specular_res;
  r.cube_vao = cube_vao;
  r.light_vao = light_vao;
  r.m_vbo = vbo;
  r.m_window = window;

  shader->use();
  shader->set_int("material.diffuse", 0);

  return r;
}

void SceneRenderer::render(input_t input, float delta) {
  auto now = glfwGetTime();
  process_events(input, delta);

  glUseProgram(p);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture_specular);
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

  set_mat4(p, "view", view);
  set_mat4(p, "projection", projection);
  set_vec3(p, "light_pos", state.light.position);
  set_vec3(p, "view_pos", state.camera.position);
  set_light(p, "light", state.light);
  set_specular_map(p, "specular_map", specular_map);

  glBindVertexArray(cube_vao);

  glUseProgram(light_id);

  set_mat4(light_id, "view", view);
  set_mat4(light_id, "projection", projection);
  set_light(light_id, "light", state.light);

  float angle;
  for (unsigned int i = 0; i < 10; ++i) {
    angle = 20.f * i;
    angle = glfwGetTime() * (i % 3) * 25.f;
    model = glm::translate(glm::mat4(1.f), example_cube_positions[i]);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));

    glUseProgram(p);
    set_mat4(p, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  model = glm::mat4(1.f);
  model = glm::translate(model, state.light.position);
  model = glm::scale(model, glm::vec3(.2f));

  glUseProgram(light_id);
  set_mat4(light_id, "model", model);
  glDrawArrays(GL_TRIANGLES, 0, 36);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
  ImGui::Begin("Scene information", NULL, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::PushItemWidth(150.0f);

  ImGui::LabelText("Pos", "(%.2f, %.2f, %.2f)", state.camera.position.x,
                   state.camera.position.y, state.camera.position.z);

  ImGui::PopItemWidth();
  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

SceneRenderer::~SceneRenderer() {
  if (!m_vbo)
    return;
  glDeleteVertexArrays(1, &cube_vao);
  glDeleteVertexArrays(1, &light_vao);
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
  if (input.light_up) {
    state.light.position += glm::vec3(0.f, 0.f, SPEED * delta);
  }
  if (input.light_down) {
    state.light.position -= glm::vec3(0.f, 0.f, SPEED * delta);
  }
  if (input.light_up) {
    state.light.position += glm::vec3(0.f, SPEED * delta, 0.f);
  }
  if (input.light_down) {
    state.light.position -= glm::vec3(0.f, SPEED * delta, 0.f);
  }
  if (input.light_left) {
    state.light.position -= glm::vec3(SPEED * delta, 0.f, 0.f);
  }
  if (input.light_right) {
    state.light.position += glm::vec3(SPEED * delta, 0.f, 0.f);
  }
  if (input.light_forward) {
    state.light.position -= glm::vec3(0.f, 0.f, SPEED * delta);
  }
  if (input.light_back) {
    state.light.position += glm::vec3(0.f, 0.f, SPEED * delta);
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
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    input.fov_inc = true;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    input.fov_dec = true;
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    input.cam_up = true;
  }
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
    input.cam_down = true;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    input.cam_left = true;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    input.cam_right = true;
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    input.cam_forward = true;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    input.cam_back = true;
  }
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    input.cam_yaw_left = true;
  }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    input.cam_yaw_right = true;
  }
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

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {}

const float MOUSE_SENSITIVITY = .1f;

void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureMouse) {
    return;
  }

  static float x = state.viewport.width / 2;
  static float y = state.viewport.height / 2;

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
  state.camera.update_fov(static_cast<float>(y_offset));
}
