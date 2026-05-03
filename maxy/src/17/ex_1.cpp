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

struct preset_t {
  std::string name;
  glm::vec4 clear_color;
  light_directional_t dir_light;
  std::array<light_positional_t, 4> pos_lights;
  std::array<light_spot_t, 2> spot_lights;
};

const std::array<preset_t, 4> presets = {
    {
        {
            .name = "desert",
            .clear_color = glm::vec4(.75f, .52f, .3f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.3f, .24f, .14f),
                    .diffuse = glm::vec3(.7f, .42f, .26f),
                    .specular = glm::vec3(.5f, .5f, .5f),
                },
            .pos_lights = {{
                {.position = glm::vec3(.7f, .2f, 2.f),
                 .ambient = glm::vec3(.1f, .06f, 0.f),
                 .diffuse = glm::vec3(1.f, .6f, 0.f),
                 .specular = glm::vec3(1.f, .6f, 0.f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(2.3f, -3.3f, -4.f),
                 .ambient = glm::vec3(.1f, 0.f, 0.f),
                 .diffuse = glm::vec3(1.f, 0.f, 0.f),
                 .specular = glm::vec3(1.f, 0.f, 0.f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(-4.f, 2.f, 12.f),
                 .ambient = glm::vec3(.1f, .1f, 0.f),
                 .diffuse = glm::vec3(1.f, 1.f, 0.f),
                 .specular = glm::vec3(1.f, 1.f, 0.f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(0.f, 0.f, -3.f),
                 .ambient = glm::vec3(.02f, .02f, .1f),
                 .diffuse = glm::vec3(.2f, .2f, 1.f),
                 .specular = glm::vec3(.2f, .2f, 1.f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
            }},
            .spot_lights = {{
                {.position = glm::vec3(1.f, 1.f, 0.f),
                 .direction = glm::vec3(0.f, 0.f, -1.f),
                 .ambient = glm::vec3(.0f, .0f, .0f),
                 .diffuse = glm::vec3(.5f, .5f, .5f),
                 .specular = glm::vec3(1.f, 1.f, 1.f),
                 .cutoff = glm::cos(glm::radians(25.f)),
                 .outer_cutoff = glm::cos(glm::radians(30.f)),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(-1.f, 1.f, 0.f),
                 .direction = glm::vec3(0.f, 0.f, -1.f),
                 .ambient = glm::vec3(.0f, .0f, .0f),
                 .diffuse = glm::vec3(.5f, .5f, .5f),
                 .specular = glm::vec3(1.f, 1.f, 1.f),
                 .cutoff = glm::cos(glm::radians(25.f)),
                 .outer_cutoff = glm::cos(glm::radians(30.f)),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
            }},
        },
        {
            .name = "factory",
            .clear_color = glm::vec4(.1f, .1f, .1f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.05f, .05f, .1f),
                    .diffuse = glm::vec3(.2f, .2f, .7f),
                    .specular = glm::vec3(.7f, .7f, .7f),
                },
            .pos_lights = {{
                {.position = glm::vec3(.7f, .2f, 2.f),
                 .ambient = glm::vec3(.02f, .02f, .06f),
                 .diffuse = glm::vec3(.2f, .2f, .6f),
                 .specular = glm::vec3(.2f, .2f, .6f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(2.3f, -3.3f, -4.f),
                 .ambient = glm::vec3(.03f, .03f, .07f),
                 .diffuse = glm::vec3(.3f, .3f, .7f),
                 .specular = glm::vec3(.3f, .3f, .7f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(-4.f, 2.f, 12.f),
                 .ambient = glm::vec3(.0f, .0f, .03f),
                 .diffuse = glm::vec3(.0f, .0f, .3f),
                 .specular = glm::vec3(.0f, .0f, .3f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(0.f, 0.f, -3.f),
                 .ambient = glm::vec3(.04f, .04f, .04f),
                 .diffuse = glm::vec3(.4f, .4f, .4f),
                 .specular = glm::vec3(.4f, .4f, .4f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
            }},
            .spot_lights = {{
                {.position = glm::vec3(1.f, 1.f, 0.f),
                 .direction = glm::vec3(0.f, 0.f, -1.f),
                 .ambient = glm::vec3(.0f, .0f, .0f),
                 .diffuse = glm::vec3(1.f, 1.f, 1.f),
                 .specular = glm::vec3(1.f, 1.f, 1.f),
                 .cutoff = glm::cos(glm::radians(25.f)),
                 .outer_cutoff = glm::cos(glm::radians(30.f)),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(-1.f, 1.f, 0.f),
                 .direction = glm::vec3(0.f, 0.f, -1.f),
                 .ambient = glm::vec3(.0f, .0f, .0f),
                 .diffuse = glm::vec3(1.f, 1.f, 1.f),
                 .specular = glm::vec3(1.f, 1.f, 1.f),
                 .cutoff = glm::cos(glm::radians(25.f)),
                 .outer_cutoff = glm::cos(glm::radians(30.f)),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
            }},
        },
        {
            .name = "horror",
            .clear_color = glm::vec4(.0f, .0f, .0f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.0f, .0f, .0f),
                    .diffuse = glm::vec3(.05f, .05f, .05f),
                    .specular = glm::vec3(.2f, .2f, .2f),
                },
            .pos_lights = {{
                {.position = glm::vec3(.7f, .2f, 2.f),
                 .ambient = glm::vec3(.01f, .01f, .01f),
                 .diffuse = glm::vec3(.1f, .1f, .1f),
                 .specular = glm::vec3(.1f, .1f, .1f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(2.3f, -3.3f, -4.f),
                 .ambient = glm::vec3(.01f, .01f, .01f),
                 .diffuse = glm::vec3(.1f, .1f, .1f),
                 .specular = glm::vec3(.1f, .1f, .1f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(-4.f, 2.f, 12.f),
                 .ambient = glm::vec3(.01f, .01f, .01f),
                 .diffuse = glm::vec3(.1f, .1f, .1f),
                 .specular = glm::vec3(.1f, .1f, .1f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(0.f, 0.f, -3.f),
                 .ambient = glm::vec3(.03f, .01f, .01f),
                 .diffuse = glm::vec3(.3f, .1f, .1f),
                 .specular = glm::vec3(.3f, .1f, .1f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
            }},
            .spot_lights = {{
                {.position = glm::vec3(1.f, 1.f, 0.f),
                 .direction = glm::vec3(0.f, 0.f, -1.f),
                 .ambient = glm::vec3(.0f, .0f, .0f),
                 .diffuse = glm::vec3(1.f, 1.f, 1.f),
                 .specular = glm::vec3(1.f, 1.f, 1.f),
                 .cutoff = glm::cos(glm::radians(25.f)),
                 .outer_cutoff = glm::cos(glm::radians(30.f)),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(-1.f, 1.f, 0.f),
                 .direction = glm::vec3(0.f, 0.f, -1.f),
                 .ambient = glm::vec3(.0f, .0f, .0f),
                 .diffuse = glm::vec3(1.f, 1.f, 1.f),
                 .specular = glm::vec3(1.f, 1.f, 1.f),
                 .cutoff = glm::cos(glm::radians(25.f)),
                 .outer_cutoff = glm::cos(glm::radians(30.f)),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
            }},
        },
        {
            .name = "biochemical",
            .clear_color = glm::vec4(.9f, .9f, .9f, 1.f),
            .dir_light =
                {
                    .direction = glm::vec3(-.2f, -1.f, -.3f),
                    .ambient = glm::vec3(.5f, .5f, .5f),
                    .diffuse = glm::vec3(1.f, 1.f, 1.f),
                    .specular = glm::vec3(1.f, 1.f, 1.f),
                },
            .pos_lights = {{
                {.position = glm::vec3(.7f, .2f, 2.f),
                 .ambient = glm::vec3(.04f, .07f, .01f),
                 .diffuse = glm::vec3(.4f, .7f, .1f),
                 .specular = glm::vec3(.4f, .7f, .1f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(2.3f, -3.3f, -4.f),
                 .ambient = glm::vec3(.04f, .07f, .01f),
                 .diffuse = glm::vec3(.4f, .7f, .1f),
                 .specular = glm::vec3(.4f, .7f, .1f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(-4.f, 2.f, 12.f),
                 .ambient = glm::vec3(.04f, .07f, .01f),
                 .diffuse = glm::vec3(.4f, .7f, .1f),
                 .specular = glm::vec3(.4f, .7f, .1f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
                {.position = glm::vec3(0.f, 0.f, -3.f),
                 .ambient = glm::vec3(.04f, .07f, .01f),
                 .diffuse = glm::vec3(.4f, .7f, .1f),
                 .specular = glm::vec3(.4f, .7f, .1f),
                 .constant = 1.f,
                 .linear = .09f,
                 .quadratic = .032f},
            }},
            .spot_lights = {{
                {.position = glm::vec3(1.f, 1.f, 0.f),
                 .direction = glm::vec3(0.f, 0.f, -1.f),
                 .ambient = glm::vec3(.0f, .0f, .0f),
                 .diffuse = glm::vec3(.0f, 1.f, .0f),
                 .specular = glm::vec3(.0f, 1.f, .0f),
                 .cutoff = glm::cos(glm::radians(25.f)),
                 .outer_cutoff = glm::cos(glm::radians(30.f)),
                 .constant = 1.f,
                 .linear = .07f,
                 .quadratic = .017f},
                {.position = glm::vec3(-1.f, 1.f, 0.f),
                 .direction = glm::vec3(0.f, 0.f, -1.f),
                 .ambient = glm::vec3(.0f, .0f, .0f),
                 .diffuse = glm::vec3(.0f, 1.f, .0f),
                 .specular = glm::vec3(.0f, 1.f, .0f),
                 .cutoff = glm::cos(glm::radians(25.f)),
                 .outer_cutoff = glm::cos(glm::radians(30.f)),
                 .constant = 1.f,
                 .linear = .07f,
                 .quadratic = .017f},
            }},
        },
    }};

struct state_t {
  window_state_t ws = {.viewport = {.width = WIDTH, .height = HEIGHT}};
  size_t preset_index = 0;
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

  programs_t m_programs{};
  vaos_t m_vaos{};
  textures_t m_textures{};
  id_t m_vbo{};
  id_t m_pyramid_vbo{};
  id_t m_pyramid_ebo{};
  GLFWwindow *m_window{};

  SceneRenderer() = default;

  static SceneRenderer setup_gl(GLFWwindow *window, Shader shader,
                                Shader light_shader, id_t diffuse,
                                id_t specular);
  void render_scene();
  void render_scene_bind_textures(const preset_t &preset, const glm::mat4 &view,
                                  const glm::mat4 &projection);
  void render_scene_draw_lights(const preset_t &preset, const glm::mat4 &view,
                                const glm::mat4 &projection);
  void render_scene_draw_cubes();
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
  struct build_t {
    Shader shader;
    Shader light_shader;
    id_t diffuse{};
    id_t specular{};
  };

  return Shader::build("shaders/17_multiple.vert", "shaders/17_multiple.frag")
      .transform([](Shader s) { return build_t{.shader = std::move(s)}; })
      .and_then([](build_t b) {
        return Shader::build("shaders/17_light.vert", "shaders/17_light.frag")
            .transform([b = std::move(b)](Shader ls) mutable {
              b.light_shader = std::move(ls);
              return b;
            });
      })
      .and_then([](build_t b) {
        return load_texture("textures/container2.png")
            .transform([b = std::move(b)](id_t d) mutable {
              b.diffuse = d;
              return b;
            });
      })
      .and_then([](build_t b) {
        return load_texture("textures/container2_specular.png")
            .transform([b = std::move(b)](id_t s) mutable {
              b.specular = s;
              return b;
            });
      })
      .transform([&](build_t b) {
        return setup_gl(window, std::move(b.shader), std::move(b.light_shader),
                        b.diffuse, b.specular);
      });
}

SceneRenderer SceneRenderer::setup_gl(GLFWwindow *window, Shader shader,
                                      Shader light_shader, id_t diffuse,
                                      id_t specular) {
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

  shader.use();
  shader.set_int("material.diffuse", 0);

  SceneRenderer r;
  r.m_programs = {.view = shader.program_id(), .light = light_shader.program_id()};
  r.m_vaos = {.cube = cube_vao, .pyramid = pyramid_vao, .light = light_vao};
  r.m_textures = {.diffuse = diffuse, .specular = specular};
  r.m_vbo = vbo;
  r.m_pyramid_vbo = pyramid_vbo;
  r.m_pyramid_ebo = pyramid_ebo;
  r.m_window = window;
  return r;
}

void SceneRenderer::render_scene() {
  const preset_t &preset = presets[state.preset_index];
  glm::mat4 view = state.ws.camera.get_view_matrix();
  glm::mat4 projection =
      glm::perspective(glm::radians(state.ws.camera.fov),
                       static_cast<float>(state.ws.viewport.width) /
                           static_cast<float>(state.ws.viewport.height),
                       .1f, 100.f);

  render_scene_bind_textures(preset, view, projection);
  render_scene_draw_lights(preset, view, projection);
  render_scene_draw_cubes();
}

void SceneRenderer::render_scene_bind_textures(const preset_t &preset,
                                               const glm::mat4 &view,
                                               const glm::mat4 &projection) {
  glUseProgram(m_programs.view);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_textures.diffuse);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_textures.specular);
  set_mat4(m_programs.view, "view", view);
  set_mat4(m_programs.view, "projection", projection);
  set_vec3(m_programs.view, "view_pos", state.ws.camera.position);
  set_specular_map(m_programs.view, "material",
                   {.diffuse = 0, .specular = 1, .shininess = 64.f});
  set_directional_light(m_programs.view, "dir_light", preset.dir_light);
  for (unsigned int i = 0; i < preset.pos_lights.size(); ++i)
    set_positional_light(m_programs.view, std::format("pos_lights[{}]", i),
                         preset.pos_lights[i]);
  for (unsigned int i = 0; i < preset.spot_lights.size(); ++i)
    set_spot_light(m_programs.view, std::format("spot_lights[{}]", i),
                   preset.spot_lights[i]);
}

void SceneRenderer::render_scene_draw_lights(const preset_t &preset,
                                             const glm::mat4 &view,
                                             const glm::mat4 &projection) {
  glUseProgram(m_programs.light);
  set_mat4(m_programs.light, "view", view);
  set_mat4(m_programs.light, "projection", projection);

  glBindVertexArray(m_vaos.light);
  for (unsigned int i = 0; i < preset.pos_lights.size(); ++i) {
    const light_positional_t &pos_light = preset.pos_lights[i];
    glm::mat4 model = glm::scale(
        glm::translate(glm::mat4(1.f), pos_light.position), glm::vec3(.2f));
    set_mat4(m_programs.light, "model", model);
    set_positional_light(m_programs.light, std::format("pos_lights[{}]", i),
                         pos_light);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  glBindVertexArray(m_vaos.pyramid);
  for (unsigned int i = 0; i < preset.spot_lights.size(); ++i) {
    const light_spot_t &spot_light = preset.spot_lights[i];
    glm::mat4 look_at =
        glm::lookAt(glm::vec3(0.f), spot_light.direction, glm::vec3(0, 1, 0));
    glm::mat4 model =
        glm::scale(glm::translate(glm::mat4(1.f), spot_light.position) *
                       glm::inverse(look_at),
                   glm::vec3(.2f));
    set_mat4(m_programs.light, "model", model);
    set_spot_light(m_programs.light, "light", spot_light);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
  }
}

void SceneRenderer::render_scene_draw_cubes() {
  glUseProgram(m_programs.view);
  glBindVertexArray(m_vaos.cube);
  for (unsigned int i = 0; i < 10; ++i) {
    float angle = glfwGetTime() * (i % 3) * 25.f;
    glm::mat4 model =
        glm::rotate(glm::translate(glm::mat4(1.f), example_cube_positions[i]),
                    glm::radians(angle), glm::vec3(1.f, .3f, .5f));
    set_mat4(m_programs.view, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
}

void SceneRenderer::render_imgui() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiIO &io = ImGui::GetIO();
  bool cam_mode = glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;

  if (cam_mode) {
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  } else {
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

void SceneRenderer::render(input_t input, float delta) {
  process_camera_events(state.ws, input, delta);

  const preset_t &preset = presets[state.preset_index];
  glClearColor(preset.clear_color.x, preset.clear_color.y, preset.clear_color.z,
               preset.clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  render_scene();

  render_imgui();
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
