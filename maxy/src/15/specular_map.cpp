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

class SceneRenderer {
public:
  static std::expected<SceneRenderer, std::string> create(GLFWwindow *window);

  SceneRenderer(const SceneRenderer &) = delete;
  SceneRenderer &operator=(const SceneRenderer &) = delete;
  SceneRenderer(SceneRenderer &&o) noexcept
      : m_ps(std::exchange(o.m_ps, {})), m_vs(std::exchange(o.m_vs, {})),
        m_ts(std::exchange(o.m_ts, {})), m_vbo(std::exchange(o.m_vbo, 0)),
        m_window(std::exchange(o.m_window, nullptr)) {}
  SceneRenderer &operator=(SceneRenderer &&) = delete;

  ~SceneRenderer();

  void render(input_t input, float delta);

private:
  struct programs_t {
    id_t view{};
    id_t light{};
  };
  struct vaos_t {
    id_t cube{};
    id_t light{};
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

  SceneRenderer() = default;

  void render_scene();
  void render_imgui();
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
  r.m_ps = {.view = shader->ID, .light = light_shader->ID};
  r.m_vs = {.cube = cube_vao, .light = light_vao};
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
  glClearColor(.2f, .3f, .3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  process_events(input, delta);
  render_scene();
  render_imgui();
}

void SceneRenderer::render_scene() {
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

  glm::mat4 view = state.ws.camera.get_view_matrix();

  glm::mat4 projection =
      glm::perspective(glm::radians(state.ws.camera.fov),
                       static_cast<float>(state.ws.viewport.width) /
                           static_cast<float>(state.ws.viewport.height),
                       .1f, 100.f);

  set_mat4(m_ps.view, "view", view);
  set_mat4(m_ps.view, "projection", projection);
  set_vec3(m_ps.view, "light_pos", state.light.position);
  set_vec3(m_ps.view, "view_pos", state.ws.camera.position);
  set_light(m_ps.view, "light", state.light);
  set_specular_map(m_ps.view, "specular_map", specular_map);

  glBindVertexArray(m_vs.cube);

  glUseProgram(m_ps.light);

  set_mat4(m_ps.light, "view", view);
  set_mat4(m_ps.light, "projection", projection);
  set_light(m_ps.light, "light", state.light);

  float angle;
  for (unsigned int i = 0; i < 10; ++i) {
    angle = glfwGetTime() * (i % 3) * 25.f;
    model = glm::translate(glm::mat4(1.f), example_cube_positions[i]);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));

    glUseProgram(m_ps.view);
    set_mat4(m_ps.view, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  model = glm::mat4(1.f);
  model = glm::translate(model, state.light.position);
  model = glm::scale(model, glm::vec3(.2f));

  glUseProgram(m_ps.light);
  set_mat4(m_ps.light, "model", model);
  glDrawArrays(GL_TRIANGLES, 0, 36);
}

void SceneRenderer::render_imgui() {
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
  glDeleteVertexArrays(1, &m_vs.cube);
  glDeleteVertexArrays(1, &m_vs.light);
  glDeleteBuffers(1, &m_vbo);
}

void process_events(input_t input, float delta) {
  process_camera_events(state.ws, input, delta);
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
