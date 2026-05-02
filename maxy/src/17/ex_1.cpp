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

struct attenuation_t {
  float constant;
  float linear;
  float quadratic;
};

struct preset_t {
  std::string name;
  glm::vec4 clear_color;
  light_directional_t dir_light;
  light_spot_t spot_light;
  std::array<glm::vec3, 4> pos_lights_pos;
  std::array<glm::vec3, 4> pos_lights_color;
  attenuation_t pos_lights_attenuation;
  std::array<glm::vec3, 2> spot_lights_pos;
  std::array<glm::vec3, 2> spot_lights_dir;
};

const std::array<preset_t, 4> presets = {
    {
        {
            .name = "desert",
            .clear_color = glm::vec4(.75f, .52f, .3f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.3f, 0.24f, .14f),
                    .diffuse = glm::vec3(.7f, .42f, .26f),
                    .specular = glm::vec3(.5f, .5f, .5f),
                },
            .spot_light =
                {
                    .ambient = glm::vec3(.0f, .0f, .0f),
                    .diffuse = glm::vec3(.5f, .5f, .5f),
                    .specular = glm::vec3(1.f, 1.f, 1.f),
                    .cutoff = glm::cos(glm::radians(25.f)),
                    .outer_cutoff = glm::cos(glm::radians(30.f)),
                    .constant = 1.f,
                    .linear = .09f,
                    .quadratic = .032f,
                },
            .pos_lights_pos = {{
                glm::vec3(.7f, .2f, 2.f),
                glm::vec3(2.3f, -3.3f, -4.f),
                glm::vec3(-4.f, 2.f, 12.f),
                glm::vec3(0.f, 0.f, -3.f),
            }},
            .pos_lights_color = {{
                glm::vec3(1.f, .6f, 0.f),
                glm::vec3(1.f, 0.f, 0.f),
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(.2f, .2f, 1.f),
            }},
            .pos_lights_attenuation = {.constant = 1.f,
                                       .linear = .09f,
                                       .quadratic = .032f},
            .spot_lights_pos = {{
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(-1.f, 1.f, 0.f),
            }},
            .spot_lights_dir = {{
                glm::vec3(0.f, 0.f, -1.f),
                glm::vec3(0.f, 0.f, -1.f),
            }},
        },
        {
            .name = "factory",
            .clear_color = glm::vec4(.1f, .1f, .1f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.05f, 0.05f, .1f),
                    .diffuse = glm::vec3(.2f, .2f, .7f),
                    .specular = glm::vec3(.7f, .7f, .7f),
                },
            .spot_light =
                {
                    .ambient = glm::vec3(.0f, .0f, .0f),
                    .diffuse = glm::vec3(1.f, 1.f, 1.f),
                    .specular = glm::vec3(1.f, 1.f, 1.f),
                    .cutoff = glm::cos(glm::radians(25.f)),
                    .outer_cutoff = glm::cos(glm::radians(30.f)),
                    .constant = 1.f,
                    .linear = .09f,
                    .quadratic = .032f,
                },
            .pos_lights_pos = {{
                glm::vec3(.7f, .2f, 2.f),
                glm::vec3(2.3f, -3.3f, -4.f),
                glm::vec3(-4.f, 2.f, 12.f),
                glm::vec3(0.f, 0.f, -3.f),
            }},
            .pos_lights_color = {{
                glm::vec3(.2f, .2f, .6f),
                glm::vec3(.3f, .3f, .7f),
                glm::vec3(.0f, .0f, .3f),
                glm::vec3(.4f, .4f, .4f),
            }},
            .pos_lights_attenuation = {.constant = 1.f,
                                       .linear = .09f,
                                       .quadratic = .032f},
            .spot_lights_pos = {{
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(-1.f, 1.f, 0.f),
            }},
            .spot_lights_dir = {{
                glm::vec3(0.f, 0.f, -1.f),
                glm::vec3(0.f, 0.f, -1.f),
            }},
        },
        {
            .name = "horror",
            .clear_color = glm::vec4(.0f, .0f, .0f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.0f, 0.0f, .0f),
                    .diffuse = glm::vec3(.05f, .05f, .05f),
                    .specular = glm::vec3(.2f, .2f, .2f),
                },
            .spot_light =
                {
                    .ambient = glm::vec3(.0f, .0f, .0f),
                    .diffuse = glm::vec3(1.f, 1.f, 1.f),
                    .specular = glm::vec3(1.f, 1.f, 1.f),
                    .cutoff = glm::cos(glm::radians(25.f)),
                    .outer_cutoff = glm::cos(glm::radians(30.f)),
                    .constant = 1.f,
                    .linear = .09f,
                    .quadratic = .032f,
                },
            .pos_lights_pos = {{
                glm::vec3(.7f, .2f, 2.f),
                glm::vec3(2.3f, -3.3f, -4.f),
                glm::vec3(-4.f, 2.f, 12.f),
                glm::vec3(0.f, 0.f, -3.f),
            }},
            .pos_lights_color = {{
                glm::vec3(.1f, .1f, .1f),
                glm::vec3(.1f, .1f, .1f),
                glm::vec3(.1f, .1f, .1f),
                glm::vec3(.3f, .1f, .1f),
            }},
            .pos_lights_attenuation = {.constant = 1.f,
                                       .linear = .09f,
                                       .quadratic = .032f},
            .spot_lights_pos = {{
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(-1.f, 1.f, 0.f),
            }},
            .spot_lights_dir = {{
                glm::vec3(0.f, 0.f, -1.f),
                glm::vec3(0.f, 0.f, -1.f),
            }},
        },
        {
            .name = "biochemical",
            .clear_color = glm::vec4(.9f, .9f, .9f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.5f, 0.5f, .5f),
                    .diffuse = glm::vec3(1.f, 1.f, 1.f),
                    .specular = glm::vec3(1.f, 1.f, 1.f),
                },
            .spot_light =
                {
                    .ambient = glm::vec3(.0f, .0f, .0f),
                    .diffuse = glm::vec3(.0f, 1.f, .0f),
                    .specular = glm::vec3(.0f, 1.f, .0f),
                    .cutoff = glm::cos(glm::radians(25.f)),
                    .outer_cutoff = glm::cos(glm::radians(30.f)),
                    .constant = 1.f,
                    .linear = .07f,
                    .quadratic = .017f,
                },
            .pos_lights_pos = {{
                glm::vec3(.7f, .2f, 2.f),
                glm::vec3(2.3f, -3.3f, -4.f),
                glm::vec3(-4.f, 2.f, 12.f),
                glm::vec3(0.f, 0.f, -3.f),
            }},
            .pos_lights_color = {{
                glm::vec3(.4f, .7f, .1f),
                glm::vec3(.4f, .7f, .1f),
                glm::vec3(.4f, .7f, .1f),
                glm::vec3(.4f, .7f, .1f),
            }},
            .pos_lights_attenuation = {.constant = 1.f,
                                       .linear = .09f,
                                       .quadratic = .032f},
            .spot_lights_pos = {{
                glm::vec3(1.f, 1.f, 0.f),
                glm::vec3(-1.f, 1.f, 0.f),
            }},
            .spot_lights_dir = {{
                glm::vec3(0.f, 0.f, -1.f),
                glm::vec3(0.f, 0.f, -1.f),
            }},
        },
    }};

struct state_t {
  window_state_t ws = {.viewport = {.width = WIDTH, .height = HEIGHT}};
  size_t preset_index = 0;};
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
        m_pyramid_vbo(std::exchange(o.m_pyramid_vbo, 0)),
        m_pyramid_ebo(std::exchange(o.m_pyramid_ebo, 0)),
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
    id_t pyramid{};
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

void process_events(input_t input, float delta);

light_positional_t get_pos_light(size_t i) {
  const preset_t &preset = presets[state.preset_index];
  return {
      .position = preset.pos_lights_pos[i],
      .ambient = preset.pos_lights_color[i] * .1f,
      .diffuse = preset.pos_lights_color[i],
      .specular = preset.pos_lights_color[i],
      .constant = preset.pos_lights_attenuation.constant,
      .linear = preset.pos_lights_attenuation.linear,
      .quadratic = preset.pos_lights_attenuation.quadratic,
  };
}

light_spot_t get_spot_light(size_t i) {
  const preset_t &preset = presets[state.preset_index];
  light_spot_t spot_light = preset.spot_light;
  spot_light.position = preset.spot_lights_pos[i];
  spot_light.direction = preset.spot_lights_dir[i];
  return spot_light;
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
  r.m_ps = {.view = shader->ID, .light = light_shader->ID};
  r.m_vs = {.cube = cube_vao, .pyramid = pyramid_vao, .light = light_vao};
  r.m_ts = {
      .diffuse = *load_texture_res,
      .specular = *load_texture_specular_res,
  };
  r.m_vbo = vbo;
  r.m_pyramid_vbo = pyramid_vbo;
  r.m_pyramid_ebo = pyramid_ebo;
  r.m_window = window;

  shader->use();
  shader->set_int("material.diffuse", 0);

  return r;
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

  glUseProgram(m_ps.view);
  set_mat4(m_ps.view, "view", view);
  set_mat4(m_ps.view, "projection", projection);
  set_vec3(m_ps.view, "view_pos", state.ws.camera.position);
  set_specular_map(m_ps.view, "material", specular_map);

  glUseProgram(m_ps.light);
  set_mat4(m_ps.light, "view", view);
  set_mat4(m_ps.light, "projection", projection);
  glBindVertexArray(m_vs.light);

  const preset_t &preset = presets[state.preset_index];
  const light_directional_t &dir_light = preset.dir_light;
  glUseProgram(m_ps.view);
  set_directional_light(m_ps.view, "dir_light", dir_light);

  // Set and draw positional lights
  for (unsigned int i = 0; i < preset.pos_lights_pos.size(); ++i) {
    light_positional_t pos_light = get_pos_light(i);
    glUseProgram(m_ps.view);
    set_positional_light(m_ps.view, std::format("pos_lights[{}]", i),
                         pos_light);
    glUseProgram(m_ps.light);
    model = glm::mat4(1.f);
    model = glm::translate(model, pos_light.position);
    model = glm::scale(model, glm::vec3(.2f));

    set_mat4(m_ps.light, "model", model);
    set_positional_light(m_ps.light, std::format("pos_lights[{}]", i),
                         pos_light);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  glUseProgram(m_ps.light);
  glBindVertexArray(m_vs.pyramid);

  // Set and draw spot lights
  for (unsigned int i = 0; i < preset.spot_lights_pos.size(); ++i) {
    light_spot_t spot_light = get_spot_light(i);
    glUseProgram(m_ps.view);
    set_spot_light(m_ps.view, std::format("spot_lights[{}]", i), spot_light);

    glUseProgram(m_ps.light);
    model = glm::mat4(1.f);
    model = glm::translate(model, spot_light.position);
    glm::mat4 look_at_rotation =
        glm::lookAt(glm::vec3(0.f), spot_light.direction, glm::vec3(0, 1, 0));
    model = model * glm::inverse(look_at_rotation);
    model = glm::scale(model, glm::vec3(.2f));

    set_mat4(m_ps.light, "model", model);
    set_spot_light(m_ps.light, "light", spot_light);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
  }

  glUseProgram(m_ps.view);
  glBindVertexArray(m_vs.cube);
  float angle;
  for (unsigned int i = 0; i < 10; ++i) {
    angle = 20.f * i;
    angle = glfwGetTime() * (i % 3) * 25.f;
    model = glm::translate(glm::mat4(1.f), example_cube_positions[i]);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));

    set_mat4(m_ps.view, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
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
  if (mode == mode_GUI) {
  }
  ImGui::PopItemWidth();

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SceneRenderer::render(input_t input, float delta) {
  process_events(input, delta);

  const preset_t &preset = presets[state.preset_index];
  glClearColor(preset.clear_color.x, preset.clear_color.y,
               preset.clear_color.z, preset.clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  render_scene();

  render_imgui();
}

SceneRenderer::~SceneRenderer() {
  if (!m_vbo)
    return;
  glDeleteVertexArrays(1, &m_vs.cube);
  glDeleteVertexArrays(1, &m_vs.light);
  glDeleteVertexArrays(1, &m_vs.pyramid);
  glDeleteBuffers(1, &m_vbo);
  glDeleteBuffers(1, &m_pyramid_vbo);
  glDeleteBuffers(1, &m_pyramid_ebo);
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
