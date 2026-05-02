#include <cmath>
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

struct state_t {
  window_state_t ws = {.viewport = {.width = WIDTH, .height = HEIGHT}};
  light_t light = {.position = glm::vec3(2.f, 1.f, -2.f),
                   .ambient = glm::vec3(.2f, .2f, .2f),
                   .diffuse = glm::vec3(.5f, .5f, .5f),
                   .specular = glm::vec3(1.f, 1.f, 1.f)};
};
// Global state
state_t state;

struct SceneRenderer {
  id_t p{};
  id_t light_id{};
  id_t cube_vao{};
  id_t light_vao{};
  id_t texture{};
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
        texture(std::exchange(o.texture, 0)), m_vbo(std::exchange(o.m_vbo, 0)),
        m_window(std::exchange(o.m_window, nullptr)) {}
  SceneRenderer &operator=(SceneRenderer &&) = delete;
};

void process_input(GLFWwindow *window, input_t &input);

int main() {
  auto ctx = GLContext::create(WIDTH, HEIGHT, TITLE);
  if (!ctx) {
    std::cerr << ctx.error() << "\n";
    return -1;
  }

  init_window_callbacks(ctx->window(), state.ws);

  auto renderer = SceneRenderer::create(ctx->window());
  if (!renderer) {
    std::cerr << renderer.error() << "\n";
    return -1;
  }
  event_loop(ctx->window(), *renderer, process_input);
  return 0;
}

void process_events(input_t input, float delta);
void strobe_light(light_t &light, double now);

std::expected<SceneRenderer, std::string>
SceneRenderer::create(GLFWwindow *window) {
  auto shader = Shader::build("shaders/15_view.vert", "shaders/15_view.frag");
  if (!shader) {
    return std::unexpected(shader.error());
  }
  auto light_shader =
      Shader::build("shaders/15_light.vert", "shaders/15_light.frag");
  if (!light_shader) {
    return std::unexpected(light_shader.error());
  }
  const std::string filename{"textures/container2.png"};
  auto load_texture_res = load_texture(filename);
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
  r.cube_vao = cube_vao;
  r.light_vao = light_vao;
  r.m_vbo = vbo;
  r.m_window = window;

  shader->use();
  shader->set_int("material.diffuse", 0);

  return r;
}

void SceneRenderer::render(input_t input, float delta) {
  glClearColor(.2f, .3f, .3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  auto now = glfwGetTime();
  process_events(input, delta);

  strobe_light(state.light, now);

  glUseProgram(p);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  diffuse_map_t diffuse_map = {
      .diffuse = 0,
      .specular = glm::vec3(.5f, .5f, .5f),
      .shininess = 64.f,
  };

  glm::mat4 model;

  glm::mat4 view = state.ws.camera.get_view_matrix();

  glm::mat4 projection =
      glm::perspective(glm::radians(state.ws.camera.fov),
                       static_cast<float>(state.ws.viewport.width) /
                           static_cast<float>(state.ws.viewport.height),
                       .1f, 100.f);

  set_mat4(p, "view", view);
  set_mat4(p, "projection", projection);
  set_vec3(p, "light_pos", state.light.position);
  set_vec3(p, "view_pos", state.ws.camera.position);
  set_light(p, "light", state.light);
  set_diffuse_map(p, "diffuse_map", diffuse_map);

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

  ImGui::LabelText("Pos", "(%.2f, %.2f, %.2f)", state.ws.camera.position.x,
                   state.ws.camera.position.y, state.ws.camera.position.z);

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
    state.ws.camera.update_fov(1.f);
  }
  if (input.fov_dec) {
    state.ws.camera.update_fov(-1.f);
  }
  if (input.cam_up) {
    state.ws.camera.process_movement(CameraMovement::UP, delta);
  }
  if (input.cam_down) {
    state.ws.camera.process_movement(CameraMovement::DOWN, delta);
  }
  if (input.cam_left) {
    state.ws.camera.process_movement(CameraMovement::LEFT, delta);
  }
  if (input.cam_right) {
    state.ws.camera.process_movement(CameraMovement::RIGHT, delta);
  }
  if (input.cam_forward) {
    state.ws.camera.process_movement(CameraMovement::FORWARD, delta);
  }
  if (input.cam_back) {
    state.ws.camera.process_movement(CameraMovement::BACKWARD, delta);
  }
  if (input.cam_yaw_left) {
    state.ws.camera.process_rotation(120 * -SPEED * delta, 0.f);
  }
  if (input.cam_yaw_right) {
    state.ws.camera.process_rotation(120 * SPEED * delta, 0.f);
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

void strobe_light(light_t &light, double now) {
  glm::vec3 color{sin(now) * 2.f, sin(now) * .7f, cos(now) * 1.3f};
  light.diffuse = color * glm::vec3(.5f);
  light.ambient = light.diffuse * glm::vec3(.2f);
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
