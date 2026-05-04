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
  light_directional_t dir_light = {
      .direction = glm::vec3(-.2f, -1.f, -.3f),
      .ambient = glm::vec3(.05f, 0.05f, .05f),
      .diffuse = glm::vec3(.4f, .4f, .4f),
      .specular = glm::vec3(.5f, .5f, .5f),
  };
  std::array<light_positional_t, 4> pos_lights = {{
      {
          .position = glm::vec3(0.7f, 0.2f, 2.0f),
          .ambient = glm::vec3(0.05f, 0.05f, 0.05f),
          .diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
          .specular = glm::vec3(1.f, 1.f, 1.f),
          .constant = 1.f,
          .linear = .09f,
          .quadratic = .032f,
      },
      {
          .position = glm::vec3(2.3f, -3.3f, -4.f),
          .ambient = glm::vec3(0.05f, 0.05f, 0.05f),
          .diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
          .specular = glm::vec3(1.f, 1.f, 1.f),
          .constant = 1.f,
          .linear = .09f,
          .quadratic = .032f,
      },
      {
          .position = glm::vec3(-4.f, 2.f, 12.f),
          .ambient = glm::vec3(0.05f, 0.05f, 0.05f),
          .diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
          .specular = glm::vec3(1.f, 1.f, 1.f),
          .constant = 1.f,
          .linear = .09f,
          .quadratic = .032f,
      },
      {
          .position = glm::vec3(0.f, 0.f, -3.f),
          .ambient = glm::vec3(0.05f, 0.05f, 0.05f),
          .diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
          .specular = glm::vec3(1.f, 1.f, 1.f),
          .constant = 1.f,
          .linear = .09f,
          .quadratic = .032f,
      },
  }};
  std::array<light_spot_t, 2> spot_lights = {{
      {
          .position = glm::vec3(1.f, 1.f, 0.f),
          .direction = glm::vec3(0.f, 0.f, -1.f),
          .ambient = glm::vec3(.0f, .0f, .0f),
          .diffuse = glm::vec3(.5f, .5f, .5f),
          .specular = glm::vec3(1.f, 1.f, 1.f),
          .cutoff = glm::cos(glm::radians(25.f)),
          .outer_cutoff = glm::cos(glm::radians(30.f)),
          .constant = 1.f,
          .linear = .09f,
          .quadratic = .032f,
      },
      {
          .position = glm::vec3(-1.f, 1.f, 0.f),
          .direction = glm::vec3(0.f, 0.f, -1.f),
          .ambient = glm::vec3(.2f, .2f, .2f),
          .diffuse = glm::vec3(.5f, .5f, .5f),
          .specular = glm::vec3(1.f, 1.f, 1.f),
          .cutoff = glm::cos(glm::radians(25.f)),
          .outer_cutoff = glm::cos(glm::radians(30.f)),
          .constant = 1.f,
          .linear = .09f,
          .quadratic = .032f,
      },
  }};
};
// Global state
state_t state;

class SceneRenderer {
public:
  static std::expected<SceneRenderer, std::string> create(GLFWwindow *window);

  SceneRenderer(const SceneRenderer &) = delete;
  SceneRenderer &operator=(const SceneRenderer &) = delete;
  SceneRenderer(SceneRenderer &&o) noexcept
      : m_programs(std::exchange(o.m_programs, {})),
        m_vaos(std::exchange(o.m_vaos, {})),
        m_textures(std::exchange(o.m_textures, {})),
        m_vbo(std::exchange(o.m_vbo, 0)),
        m_pyramid_vbo(std::exchange(o.m_pyramid_vbo, 0)),
        m_pyramid_ebo(std::exchange(o.m_pyramid_ebo, 0)),
        m_window(std::exchange(o.m_window, nullptr)) {}
  SceneRenderer &operator=(SceneRenderer &&) = delete;

  ~SceneRenderer();

  void render(input_t input, float delta);

private:
  struct programs_t {
    Shader view;
    Shader light;
  };
  struct vaos_t {
    id_t cube{};
    id_t pyramid{};
    id_t light{};
  };
  struct textures_t {
    id_t diffuse{};
    id_t specular{};
  };

  programs_t m_programs{};
  vaos_t m_vaos{};
  textures_t m_textures{};
  id_t m_vbo{};
  id_t m_pyramid_vbo{};
  id_t m_pyramid_ebo{};
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

std::expected<SceneRenderer, std::string>
SceneRenderer::create(GLFWwindow *window) {
  auto shader =
      Shader::build("shaders/17_multiple.vert", "shaders/17_multiple.frag");
  if (!shader) {
    return std::unexpected(shader.error());
  }
  auto light_shader =
      Shader::build("shaders/17_light.vert", "shaders/17_light.frag");
  if (!light_shader) {
    return std::unexpected(light_shader.error());
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

  id_t cube_vao;
  id_t vbo;
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

  id_t light_vao;
  glGenVertexArrays(1, &light_vao);
  glBindVertexArray(light_vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t),
      reinterpret_cast<void *>(offsetof(cube_vertex_t, position)));
  glEnableVertexAttribArray(0);

  id_t pyramid_vao;
  id_t pyramid_vbo;
  id_t pyramid_ebo;
  glGenVertexArrays(1, &pyramid_vao);
  glGenBuffers(1, &pyramid_vbo);
  glGenBuffers(1, &pyramid_ebo);
  glBindVertexArray(pyramid_vao);

  glBindBuffer(GL_ARRAY_BUFFER, pyramid_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid_vertices), pyramid_vertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramid_ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramid_indices),
               pyramid_indices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(0);

  SceneRenderer r;
  r.m_programs = {.view = std::move(*shader), .light = std::move(*light_shader)};
  r.m_vaos = {.cube = cube_vao, .pyramid = pyramid_vao, .light = light_vao};
  r.m_textures = {
      .diffuse = *load_texture_res,
      .specular = *load_texture_specular_res,
  };
  r.m_vbo = vbo;
  r.m_pyramid_vbo = pyramid_vbo;
  r.m_pyramid_ebo = pyramid_ebo;
  r.m_window = window;

  r.m_programs.view.use();
  r.m_programs.view.set_int("material.diffuse", 0);

  return r;
}

void SceneRenderer::render(input_t input, float delta) {
  glClearColor(.2f, .3f, .3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  process_camera_events(state.ws, input, delta);
  render_scene();
  render_imgui();
}

void SceneRenderer::render_scene() {
  m_programs.view.use();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_textures.diffuse);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_textures.specular);
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

  set_mat4(m_programs.view.program_id(), "view", view);
  set_mat4(m_programs.view.program_id(), "projection", projection);
  set_vec3(m_programs.view.program_id(), "view_pos", state.ws.camera.position);
  set_specular_map(m_programs.view.program_id(), "material", specular_map);
  set_directional_light(m_programs.view.program_id(), "dir_light", state.dir_light);

  glBindVertexArray(m_vaos.cube);
  for (unsigned int i = 0; i < state.pos_lights.size(); ++i) {
    set_positional_light(m_programs.view.program_id(), std::format("pos_lights[{}]", i),
                         state.pos_lights[i]);
  }
  for (unsigned int i = 0; i < state.spot_lights.size(); ++i) {
    set_spot_light(m_programs.view.program_id(), std::format("spot_lights[{}]", i),
                   state.spot_lights[i]);
  }

  m_programs.view.use();
  float angle;
  for (unsigned int i = 0; i < 10; ++i) {
    angle = glfwGetTime() * (i % 3) * 25.f;
    model = glm::translate(glm::mat4(1.f), example_cube_positions[i]);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));

    set_mat4(m_programs.view.program_id(), "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  m_programs.light.use();
  set_mat4(m_programs.light.program_id(), "view", view);
  set_mat4(m_programs.light.program_id(), "projection", projection);

  glBindVertexArray(m_vaos.light);
  for (unsigned int i = 0; i < state.pos_lights.size(); ++i) {
    model = glm::mat4(1.f);
    model = glm::translate(model, state.pos_lights[i].position);
    model = glm::scale(model, glm::vec3(.2f));

    set_mat4(m_programs.light.program_id(), "model", model);
    set_positional_light(m_programs.light.program_id(), std::format("pos_lights[{}]", i),
                         state.pos_lights[i]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  glBindVertexArray(m_vaos.pyramid);
  for (unsigned int i = 0; i < state.spot_lights.size(); ++i) {
    model = glm::mat4(1.f);
    model = glm::translate(model, state.spot_lights[i].position);
    glm::mat4 look_at_rotation = glm::lookAt(
        glm::vec3(0.f), state.spot_lights[i].direction, glm::vec3(0, 1, 0));
    model = model * glm::inverse(look_at_rotation);
    model = glm::scale(model, glm::vec3(.2f));

    set_mat4(m_programs.light.program_id(), "model", model);
    set_spot_light(m_programs.light.program_id(), "light", state.spot_lights[i]);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
  }
}

void SceneRenderer::render_imgui() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiIO &io = ImGui::GetIO();
  enum mode_t {
    mode_CAM,
    mode_GUI,
  };
  mode_t mode =
      (glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
          ? mode_CAM
          : mode_GUI;

  if (mode == mode_CAM) {
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  } else if (mode == mode_GUI) {
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
  }

  ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
  ImGui::Begin("Scene information", NULL, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::PushItemWidth(150.0f);

  ImGui::LabelText("Pos", "(%.2f, %.2f, %.2f)", state.ws.camera.position.x,
                   state.ws.camera.position.y, state.ws.camera.position.z);
  double x, y;
  glfwGetCursorPos(m_window, &x, &y);
  ImGui::LabelText("Mouse", "(%.2f, %.2f)", x, y);
  ImGui::PopItemWidth();

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

SceneRenderer::~SceneRenderer() {
  if (!m_vbo)
    return;
  glDeleteVertexArrays(1, &m_vaos.cube);
  glDeleteVertexArrays(1, &m_vaos.light);
  glDeleteVertexArrays(1, &m_vaos.pyramid);
  glDeleteBuffers(1, &m_vbo);
  glDeleteBuffers(1, &m_pyramid_vbo);
  glDeleteBuffers(1, &m_pyramid_ebo);
}

void process_input(GLFWwindow *window, input_t &input) {
  process_common_input(window, input);
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    input.light.forward = true;
  }
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    input.light.back = true;
  }
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    input.light.left = true;
  }
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    input.light.right = true;
  }
  if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
    input.light.up = true;
  }
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
    input.light.down = true;
  }
}
