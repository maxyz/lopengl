#include <cmath>
#include <expected>
#include <glm/geometric.hpp>
#include <iostream>
#include <print>
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
  view_t viewport = {
      .width = WIDTH,
      .height = HEIGHT,
  };
  Camera camera = Camera(glm::vec3(0.f, 0.f, 3.f));
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

  bool mouse_new_focus = true;
};
// Global state
state_t state;



struct SceneRenderer {
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

  programs_t ps{};
  vaos_t vs{};
  textures_t ts{};
  id_t vbo{};
  id_t pyramid_vbo{};
  id_t pyramid_ebo{};
  GLFWwindow *window{};

  static std::expected<SceneRenderer, std::string> create(GLFWwindow *window);
  void render(input_t input, float delta);

  ~SceneRenderer();
  SceneRenderer() = default;
  SceneRenderer(const SceneRenderer &) = delete;
  SceneRenderer &operator=(const SceneRenderer &) = delete;
  SceneRenderer(SceneRenderer &&o) noexcept
      : ps(std::exchange(o.ps, {})), vs(std::exchange(o.vs, {})),
        ts(std::exchange(o.ts, {})), vbo(std::exchange(o.vbo, 0)),
        pyramid_vbo(std::exchange(o.pyramid_vbo, 0)),
        pyramid_ebo(std::exchange(o.pyramid_ebo, 0)),
        window(std::exchange(o.window, nullptr)) {}
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

std::expected<SceneRenderer, std::string> SceneRenderer::create(GLFWwindow *window) {
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
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t), reinterpret_cast<void *>(offsetof(cube_vertex_t, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t), reinterpret_cast<void *>(offsetof(cube_vertex_t, normal)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t), reinterpret_cast<void *>(offsetof(cube_vertex_t, texcoord)));
  glEnableVertexAttribArray(2);

  id_t light_vao;
  glGenVertexArrays(1, &light_vao);
  glBindVertexArray(light_vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(cube_vertex_t), reinterpret_cast<void *>(offsetof(cube_vertex_t, position)));
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
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(0);

  SceneRenderer r;
  r.ps = {.view = shader->ID, .light = light_shader->ID};
  r.vs = {.cube = cube_vao, .pyramid = pyramid_vao, .light = light_vao};
  r.ts = {
      .diffuse = *load_texture_res,
      .specular = *load_texture_specular_res,
  };
  r.vbo = vbo;
  r.pyramid_vbo = pyramid_vbo;
  r.pyramid_ebo = pyramid_ebo;
  r.window = window;

  shader->use();
  shader->set_int("material.diffuse", 0);

  return r;
}

void SceneRenderer::render(input_t input, float delta) {
  auto now = glfwGetTime();
  process_events(input, delta);

  glUseProgram(ps.view);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ts.diffuse);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, ts.specular);
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

  set_mat4(ps.view, "view", view);
  set_mat4(ps.view, "projection", projection);
  set_vec3(ps.view, "view_pos", state.camera.position);
  set_specular_map(ps.view, "material", specular_map);
  set_directional_light(ps.view, "dir_light", state.dir_light);

  glBindVertexArray(vs.cube);
  for (unsigned int i = 0; i < state.pos_lights.size(); ++i) {
    set_positional_light(ps.view, std::format("pos_lights[{}]", i),
                         state.pos_lights[i]);
  }
  for (unsigned int i = 0; i < state.spot_lights.size(); ++i) {
    set_spot_light(ps.view, std::format("spot_lights[{}]", i),
                   state.spot_lights[i]);
  }

  glUseProgram(ps.view);
  float angle;
  for (unsigned int i = 0; i < 10; ++i) {
    angle = 20.f * i;
    angle = glfwGetTime() * (i % 3) * 25.f;
    model = glm::translate(glm::mat4(1.f), example_cube_positions[i]);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));

    set_mat4(ps.view, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  glUseProgram(ps.light);
  set_mat4(ps.light, "view", view);
  set_mat4(ps.light, "projection", projection);

  glBindVertexArray(vs.light);
  // Draw positional lights
  for (unsigned int i = 0; i < state.pos_lights.size(); ++i) {
    model = glm::mat4(1.f);
    model = glm::translate(model, state.pos_lights[i].position);
    model = glm::scale(model, glm::vec3(.2f));

    set_mat4(ps.light, "model", model);
    set_positional_light(ps.light, std::format("pos_lights[{}]", i),
                         state.pos_lights[i]);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  }

  glBindVertexArray(vs.pyramid);
  // Draw spot lights as pyramids
  for (unsigned int i = 0; i < state.spot_lights.size(); ++i) {
    model = glm::mat4(1.f);
    model = glm::translate(model, state.spot_lights[i].position);
    glm::mat4 look_at_rotation = glm::lookAt(
        glm::vec3(0.f), state.spot_lights[i].direction, glm::vec3(0, 1, 0));
    model = model * glm::inverse(look_at_rotation);
    model = glm::scale(model, glm::vec3(.2f));

    set_mat4(ps.light, "model", model);
    set_spot_light(ps.light, "light", state.spot_lights[i]);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiIO &io = ImGui::GetIO();
  enum mode_t {
    mode_CAM,
    mode_GUI,
  };
  mode_t mode =
      (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
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

  ImGui::LabelText("Pos", "(%.2f, %.2f, %.2f)", state.camera.position.x,
                   state.camera.position.y, state.camera.position.z);
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  ImGui::LabelText("Mouse", "(%.2f, %.2f)", x, y);
  if (mode == mode_GUI) {
  }
  ImGui::PopItemWidth();

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

SceneRenderer::~SceneRenderer() {
  if (!vbo) return;
  glDeleteVertexArrays(1, &vs.cube);
  glDeleteVertexArrays(1, &vs.light);
  glDeleteVertexArrays(1, &vs.pyramid);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &pyramid_vbo);
  glDeleteBuffers(1, &pyramid_ebo);
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
