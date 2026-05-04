#include <expected>
#include <iostream>
#include <utility>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "common/common.hpp"

const char *TITLE = "Model loading example";
const GLuint WIDTH = 1024;
const GLuint HEIGHT = 768;

struct model_preset_t {
  glm::vec3 position;
  float scale;
};
struct preset_t {
  std::string name;
  glm::vec4 clear_color;
  model_preset_t model;
};

struct state_t {
  window_state_t ws = {.viewport = {.width = WIDTH, .height = HEIGHT}};
  size_t preset_index = 0;
};
state_t state;

class SceneRenderer {
public:
  static std::expected<SceneRenderer, std::string> create(GLFWwindow *window);

  SceneRenderer(const SceneRenderer &) = delete;
  SceneRenderer &operator=(const SceneRenderer &) = delete;
  SceneRenderer(SceneRenderer &&o) noexcept = default;
  SceneRenderer &operator=(SceneRenderer &&) = default;

  void render(input_t input, float delta);

private:
  GLFWwindow *m_window{};
  Shader m_shader{};
  Model m_model;

  SceneRenderer() = default;

  static SceneRenderer setup_gl(GLFWwindow *window, Shader shader, Model model);
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

const std::array<preset_t, 4> presets = {{
    {
        .name = "simple",
        .clear_color = glm::vec4(.75f, .52f, .3f, 1.f),
        .model =
            {
                .position = glm::vec3(0.f, 0.f, 0.f),
                .scale = 1.f,
            },
    },
}};

std::expected<SceneRenderer, std::string>
SceneRenderer::create(GLFWwindow *window) {
  struct build_t {
    Shader shader;
    Model model;
  };

  return Shader::build("shaders/21_model.vert", "shaders/21_model.frag")
      .transform([](Shader s) { return build_t{.shader = std::move(s)}; })
      .and_then([](build_t b) {
        return Model::load("objects/backpack/backpack.obj")
            .transform([b = std::move(b)](Model m) mutable {
              b.model = std::move(m);
              return std::move(b);
            });
      })
      .transform([&](build_t b) {
        return setup_gl(window, std::move(b.shader), std::move(b.model));
      });
}

SceneRenderer SceneRenderer::setup_gl(GLFWwindow *window, Shader shader,
                                      Model model) {
  SceneRenderer r;
  r.m_window = window;
  r.m_shader = std::move(shader);
  r.m_model = std::move(model);
  return r;
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

void SceneRenderer::render_scene() {
  m_shader.use();
  const preset_t &preset = presets[state.preset_index];
  glm::mat4 view = state.ws.camera.get_view_matrix();
  glm::mat4 projection =
      glm::perspective(glm::radians(state.ws.camera.fov),
                       static_cast<float>(state.ws.viewport.width) /
                           static_cast<float>(state.ws.viewport.height),
                       .1f, 100.f);
  m_shader.set_mat4("view", view);
  m_shader.set_mat4("projection", projection);

  glm::mat4 model = glm::mat4(1.f);
  model = glm::translate(model, preset.model.position);
  model = glm::scale(model, preset.model.scale * glm::vec3(1.f));
  m_shader.set_mat4("model", model);

  m_model.draw(m_shader);
}

void SceneRenderer::render_imgui() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiIO &io = ImGui::GetIO();
  bool cam_mode =
      glfwGetInputMode(m_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;

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
  if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
    input.light.up = true;
  }
  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
    input.light.down = true;
  }
}
