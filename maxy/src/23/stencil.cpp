#include <expected>
#include <iostream>
#include <utility>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "common/common.hpp"

constexpr const char *TITLE = "Model loading example";
constexpr GLuint WIDTH = 1024;
constexpr GLuint HEIGHT = 768;
constexpr float BORDER_THICKNESS = 0.015f;

struct model_preset_t {
  glm::vec3 position;
  float scale;
};
struct preset_t {
  std::string name;
  glm::vec4 clear_color;
  std::vector<model_preset_t> cubes;
  model_preset_t plane;
};

struct depth_mode_t {
  std::string name;
  GLenum mode;
};

struct state_t {
  window_state_t window = {.viewport = {.width = WIDTH, .height = HEIGHT}};
  size_t preset_index = 0;
  size_t depth_mode_index = 2; // Initial value set to less
  float border_thickness = BORDER_THICKNESS;
};
state_t state;

struct shaders_t {
  Shader shader;
  Shader border;
};
struct vaos_t {
  id_t cube;
  id_t plane;
};
struct vbos_t {
  id_t cube;
  id_t plane;
};
struct textures_t {
  id_t marble;
  id_t metal;
};

class SceneRenderer {
public:
  static std::expected<std::unique_ptr<SceneRenderer>, std::string>
  create(GLFWwindow *window);

  SceneRenderer(const SceneRenderer &) = delete;
  SceneRenderer &operator=(const SceneRenderer &) = delete;
  SceneRenderer(SceneRenderer &&o) noexcept = delete;
  SceneRenderer &operator=(SceneRenderer &&o) = delete;
  ~SceneRenderer() noexcept {
    glDeleteVertexArrays(2, &m_vaos.cube);
    glDeleteBuffers(2, &m_vbos.cube);
  }

  void render(input_t input, float delta);

private:
  GLFWwindow *m_window{};
  shaders_t m_shaders;
  textures_t m_textures{};
  vaos_t m_vaos;
  vbos_t m_vbos;

  SceneRenderer(
      GLFWwindow *window, shaders_t shaders, textures_t textures, vaos_t vaos,
      vbos_t vbos
  )
      : m_window{window}, m_shaders{std::move(shaders)}, m_textures(textures),
        m_vaos{vaos}, m_vbos(vbos) {};

  void render_scene();
  void render_fill_pass();
  void render_outline_pass();
  void render_scene_set_view_and_projection();
  void render_scene_draw_cubes(Shader &);
  void render_scene_draw_plane();
  void render_imgui();
};

const std::array<preset_t, 1> presets = {{
    {
        .name = "simple",
        .clear_color = glm::vec4(.01f, .01f, .0f, 1.f),
        .cubes =
            {
                {
                    .position = glm::vec3(-1.f, 0.f, 1.f),
                    .scale = 1.f,
                },
                {
                    .position = glm::vec3(2.f, 0.f, 0.f),
                    .scale = 2.f,
                },
            },
        .plane = {
            .position = glm::vec3(0.f, 0.f, 0.f),
            .scale = 1.f,
        },
    },
}};

const std::array<depth_mode_t, 8> depth_modes = {{
    {
        .name = "always",
        .mode = GL_ALWAYS,
    },
    {
        .name = "never",
        .mode = GL_NEVER,
    },
    {
        .name = "less",
        .mode = GL_LESS,
    },
    {
        .name = "equal",
        .mode = GL_EQUAL,
    },
    {
        .name = "less equal",
        .mode = GL_LEQUAL,
    },
    {
        .name = "greater",
        .mode = GL_GREATER,
    },
    {
        .name = "not equal",
        .mode = GL_NOTEQUAL,
    },
    {
        .name = "greater equal",
        .mode = GL_GEQUAL,
    },
}};

std::expected<textures_t, std::string> load_textures() {
  auto marble_texture = load_texture("textures/marble.jpg");
  if (!marble_texture) {
    return std::unexpected(marble_texture.error());
  }
  auto metal_texture = load_texture("textures/metal.png");
  if (!metal_texture) {
    return std::unexpected(metal_texture.error());
  }
  return textures_t{.marble = *marble_texture, .metal = *metal_texture};
}

void load_buffer_vertices(
    std::span<const vertex_t> vertices, id_t vertex_array_object,
    id_t vertex_buffer_object
) {

  glBindVertexArray(vertex_array_object);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
  glBufferData(
      GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW
  );
  glVertexAttribPointer(
      0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
      reinterpret_cast<void *>(offsetof(vertex_t, position))
  );
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
      reinterpret_cast<void *>(offsetof(vertex_t, normal))
  );
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
      reinterpret_cast<void *>(offsetof(vertex_t, texcoord))
  );
  glEnableVertexAttribArray(2);
}

std::pair<vaos_t, vbos_t> load_buffers() {
  vaos_t vaos{};
  vbos_t vbos{};

  glGenVertexArrays(2, &vaos.cube);
  glGenBuffers(2, &vbos.cube);

  load_buffer_vertices(cube_vertices, vaos.cube, vbos.cube);
  load_buffer_vertices(floor_vertices, vaos.plane, vbos.plane);

  return {vaos, vbos};
}

std::expected<std::unique_ptr<SceneRenderer>, std::string>
SceneRenderer::create(GLFWwindow *window) {
  auto shader = Shader::build("shaders/23_depth.vert", "shaders/23_depth.frag");
  if (!shader) {
    return std::unexpected(shader.error());
  }
  auto border_shader =
      Shader::build("shaders/23_border.vert", "shaders/23_border.frag");
  if (!border_shader) {
    return std::unexpected(border_shader.error());
  }
  shaders_t shaders = {
      .shader = std::move(*shader),
      .border = std::move(*border_shader),
  };
  auto textures = load_textures();
  if (!textures) {
    return std::unexpected(textures.error());
  }
  shader->use();
  shader->set_int("texture1", 0);
  auto [vaos, vbos] = load_buffers();

  return std::unique_ptr<SceneRenderer>{
      new SceneRenderer{window, std::move(shaders), *textures, vaos, vbos}
  };
}

void SceneRenderer::render(input_t input, float delta) {
  process_camera_events(state.window, input, delta);

  const preset_t &preset = presets[state.preset_index];
  glClearColor(
      preset.clear_color.x, preset.clear_color.y, preset.clear_color.z,
      preset.clear_color.w
  );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  render_scene();

  render_imgui();
}

void SceneRenderer::render_scene() {
  render_scene_set_view_and_projection();
  render_fill_pass();
  render_outline_pass();
}

void SceneRenderer::render_fill_pass() {
  render_scene_draw_plane();

  glStencilFunc(GL_ALWAYS, 1, 0xFF);
  glStencilMask(0xFF);
  render_scene_draw_cubes(m_shaders.shader);
}

void SceneRenderer::render_outline_pass() {
  glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
  glStencilMask(0x00);
  glDisable(GL_DEPTH_TEST);
  m_shaders.border.use();
  m_shaders.border.set_float("border_thickness", state.border_thickness);
  render_scene_draw_cubes(m_shaders.border);
  glEnable(GL_DEPTH_TEST);
  glStencilMask(0xFF);
  glStencilFunc(GL_ALWAYS, 0, 0xFF);
}

void SceneRenderer::render_scene_set_view_and_projection() {
  glm::mat4 view = state.window.camera.get_view_matrix();
  glm::mat4 projection = glm::perspective(
      glm::radians(state.window.camera.fov),
      static_cast<float>(state.window.viewport.width) /
          static_cast<float>(state.window.viewport.height),
      .1f, 100.f
  );
  m_shaders.shader.use();
  m_shaders.shader.set_mat4("view", view);
  m_shaders.shader.set_mat4("projection", projection);
  m_shaders.border.use();
  m_shaders.border.set_mat4("view", view);
  m_shaders.border.set_mat4("projection", projection);
}

void SceneRenderer::render_scene_draw_cubes(Shader &shader) {
  const preset_t &preset = presets[state.preset_index];

  shader.use();

  glBindVertexArray(m_vaos.cube);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_textures.marble);

  for (auto &model_info : preset.cubes) {
    glm::mat4 model_transform = glm::mat4(1.f);
    model_transform = glm::translate(model_transform, model_info.position);
    model_transform = glm::scale(model_transform, glm::vec3(model_info.scale));

    shader.set_mat4("model", model_transform);
    glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
  }
}

void SceneRenderer::render_scene_draw_plane() {
  glStencilFunc(GL_ALWAYS, 0, 0xFF);
  glStencilMask(0x00);

  const preset_t &preset = presets[state.preset_index];

  glBindVertexArray(m_vaos.plane);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_textures.metal);

  auto &model_info = preset.plane;
  glm::mat4 model_transform = glm::mat4(1.f);
  model_transform = glm::translate(model_transform, model_info.position);
  model_transform = glm::scale(model_transform, glm::vec3(model_info.scale));

  m_shaders.shader.use();
  m_shaders.shader.set_mat4("model", model_transform);
  glDrawArrays(GL_TRIANGLES, 0, floor_vertices.size());
}

void SceneRenderer::render_imgui() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiIO &io = ImGui::GetIO();
  bool camera_mode =
      glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;

  if (camera_mode) {
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
  } else {
    io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
  }

  ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
  ImGui::Begin("Scene information", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::PushItemWidth(150.0f);

  ImGui::LabelText(
      "Pos", "(%.2f, %.2f, %.2f)", state.window.camera.position.x,
      state.window.camera.position.y, state.window.camera.position.z
  );
  double x, y;
  glfwGetCursorPos(m_window, &x, &y);
  ImGui::LabelText("Mouse", "(%.2f, %.2f)", x, y);
  ImGui::LabelText(
      "Dept Mode", "%s", depth_modes[state.depth_mode_index].name.c_str()
  );
  if (camera_mode) {
    ImGui::LabelText("Border", "%.4f", state.border_thickness);
  } else {
    ImGui::SliderFloat(
        "Border", &state.border_thickness, 0.0f, 0.05f, "%.4f"
    );
  }
  ImGui::PopItemWidth();

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void set_depth_func() {
  auto &depth_mode = depth_modes[state.depth_mode_index];
  glDepthFunc(depth_mode.mode);
}

void key_callback_with_depth_mode(
    GLFWwindow *window, int key, int scancode, int action, int mods
) {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureKeyboard) {
    return;
  }
  key_callback(window, key, scancode, action, mods);

  if ((key == GLFW_KEY_M) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (mods & GLFW_MOD_SHIFT) {
      state.depth_mode_index =
          (depth_modes.size() + state.depth_mode_index - 1) %
          depth_modes.size();
    } else {
      state.depth_mode_index =
          (state.depth_mode_index + 1) % depth_modes.size();
    }
    set_depth_func();
  }
}

void process_input(GLFWwindow *window, input_t &input) {
  process_common_input(window, input);
}

int main() {
  auto ctx = GLContext::create(WIDTH, HEIGHT, TITLE);
  if (!ctx) {
    std::cerr << ctx.error() << "\n";
    return -1;
  }
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

  window_callbacks_t window_callbacks{DEFAULT_WINDOW_CALLBACKS};
  window_callbacks.key = key_callback_with_depth_mode;
  init_window_callbacks(ctx->window(), state.window, window_callbacks);

  auto renderer_res = SceneRenderer::create(ctx->window());
  if (!renderer_res) {
    std::cerr << renderer_res.error() << "\n";
    return -1;
  }
  auto renderer = std::move(*renderer_res);
  set_depth_func();

  event_loop(ctx->window(), *renderer, process_input);
  return 0;
}
