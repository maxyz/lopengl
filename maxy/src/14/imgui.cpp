#include <cmath>
#include <expected>
#include <functional>
#include <glm/geometric.hpp>
#include <iostream>
#include <print>
#include <vector>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include "common/assets.hpp"
#include "common/camera.hpp"
#include "common/light.hpp"
#include "common/materials.hpp"
#include "common/shader.hpp"

const char *TITLE = "LOpenGL";
const GLuint WIDTH = 1024;
const GLuint HEIGHT = 768;

struct view_t {
  float width = WIDTH;
  float height = HEIGHT;
};
view_t viewport;

Camera camera = Camera(glm::vec3(0.f, 0.f, 3.f));

const std::vector<material_t> materials = {
    emerald,     jade,          obsidian,       pearl,        ruby,
    turquoise,   brass,         bronze,         chrome,       copper,
    gold,        silver,        black_plastic,  cyan_plastic, green_plastic,
    red_plastic, white_plastic, yellow_plastic, black_rubber, green_rubber,
    red_rubber,  white_rubber,  yellow_rubber};

size_t current_material = 0;
bool textures_enabled = true;

enum event_t {
  NONE = 0,
  increase_fov = 1 << 0,
  decrease_fov = 1 << 1,
  camera_up = 1 << 2,
  camera_down = 1 << 3,
  camera_left = 1 << 4,
  camera_right = 1 << 5,
  camera_for = 1 << 6,
  camera_back = 1 << 7,
  camera_yaw_left = 1 << 8,
  camera_yaw_right = 1 << 9,
  light_up = 1 << 10,
  light_down = 1 << 11,
  light_left = 1 << 12,
  light_right = 1 << 13,
  light_for = 1 << 14,
  light_back = 1 << 15,
};

using cb_t = std::function<void(uint64_t event, float delta)>;
using cbs_t = std::vector<cb_t>;
using cleanup_t = std::function<void()>;

struct hooks_t {
  cbs_t callbacks = {};
  cleanup_t cleanup = []() {};
};

std::expected<GLFWwindow *, std::string> init_window();
std::expected<hooks_t, std::string> init_shaders();
std::expected<void, std::string> init_textures();

void event_loop(GLFWwindow *window, cbs_t cbs);

void cleanup(cleanup_t);

int main() {
  auto window = init_window();
  if (!window) {
    std::cerr << window.error() << std::endl;
    return -1;
  }

  auto res = init_shaders();
  if (!res) {
    std::cerr << res.error() << std::endl;
    return -1;
  }
  event_loop(*window, res->callbacks);

  cleanup(res->cleanup);
  return 0;
}

void cleanup(cleanup_t cleanup_) {
  cleanup_();

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);
bool mouse_new_focus = true;
void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void window_focus_callback(GLFWwindow *window, int focused);

std::expected<GLFWwindow *, std::string> init_window() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(viewport.width, viewport.height, TITLE, NULL, NULL);
  if (window == NULL) {
    glfwTerminate();
    return std::unexpected("failed to create GLFW window");
  }
  glfwMakeContextCurrent(window);
  int version = gladLoadGL(glfwGetProcAddress);
  if (version == 0) {
    glfwTerminate();
    return std::unexpected("failed to init glad on top of glfw");
  }
  glViewport(0, 0, viewport.width, viewport.height);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  auto font_path = get_asset_path("fonts/NotoSans-Regular.ttf");
  if (!font_path) {
    glfwTerminate();
    return std::unexpected("failed to obtain default font");
  }
  io.Fonts->AddFontFromFileTTF(font_path->c_str(), 20);
  io.IniFilename = NULL;

  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  glfwSetKeyCallback(window, key_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_DEPTH_TEST);
  return window;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  viewport.width = width;
  viewport.height = height;
  glViewport(0, 0, width, height);
}

void window_focus_callback(GLFWwindow *window, int focused) {
  if (!focused) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, NULL);

  } else {
    mouse_new_focus = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
  }
}

const float h = std::sin(M_PI / 3);

const float vertices[] = {
    // positions      // normals      // texture coords
    -.5f, -.5f, -.5f, .0f,  .0f,   -1.f, .0f, .0f, // back
    .5f,  -.5f, -.5f, .0f,  .0f,   -1.f, 1.f, .0f, // back
    .5f,  .5f,  -.5f, .0f,  .0f,   -1.f, 1.f, 1.f, // back
    .5f,  .5f,  -.5f, .0f,  .0f,   -1.f, 1.f, 1.f, // back 2
    -.5f, .5f,  -.5f, .0f,  .0f,   -1.f, .0f, 1.f, // back 2
    -.5f, -.5f, -.5f, .0f,  .0f,   -1.f, .0f, .0f, // back 2

    .5f,  -.5f, .5f,  .0f,  .0f,   1.f,  .0f, .0f, // front
    -.5f, -.5f, .5f,  .0f,  .0f,   1.f,  1.f, .0f, // front
    -.5f, .5f,  .5f,  .0f,  .0f,   1.f,  1.f, 1.f, // front
    -.5f, .5f,  .5f,  .0f,  .0f,   1.f,  1.f, 1.f, // front 2
    .5f,  .5f,  .5f,  .0f,  .0f,   1.f,  .0f, 1.f, // front 2
    .5f,  -.5f, .5f,  .0f,  .0f,   1.f,  .0f, .0f, // front 2

    -.5f, -.5f, .5f,  -1.f, .0f,   .0f,  .0f, .0f, // left
    -.5f, -.5f, -.5f, -1.f, .0f,   .0f,  1.f, .0f, // left
    -.5f, .5f,  -.5f, -1.f, .0f,   .0f,  1.f, 1.f, // left
    -.5f, .5f,  -.5f, -1.f, .0f,   .0f,  1.f, 1.f, // left 2
    -.5f, .5f,  .5f,  -1.f, .0f,   .0f,  .0f, 1.f, // left 2
    -.5f, -.5f, .5f,  -1.f, .0f,   .0f,  .0f, .0f, // left 2

    .5f,  -.5f, -.5f, 1.f,  .0f,   .0f,  .0f, .0f, // right
    .5f,  -.5f, .5f,  1.f,  .0f,   .0f,  1.f, .0f, // right
    .5f,  .5f,  .5f,  1.f,  .0f,   .0f,  1.f, 1.f, // right
    .5f,  .5f,  .5f,  1.f,  .0f,   .0f,  1.f, 1.f, // right 2
    .5f,  .5f,  -.5f, 1.f,  .0f,   .0f,  .0f, 1.f, // right 2
    .5f,  -.5f, -.5f, 1.f,  .0f,   .0f,  .0f, .0f, // right 2

    -.5f, -.5f, -.5f, .0f,  -1.0f, .0f,  .0f, .0f, // bottom
    .5f,  -.5f, -.5f, .0f,  -1.0f, .0f,  1.f, .0f, // bottom
    .5f,  -.5f, .5f,  .0f,  -1.0f, .0f,  1.f, 1.f, // bottom
    .5f,  -.5f, .5f,  .0f,  -1.0f, .0f,  1.f, 1.f, // bottom 2
    -.5f, -.5f, .5f,  .0f,  -1.0f, .0f,  .0f, 1.f, // bottom 2
    -.5f, -.5f, -.5f, .0f,  -1.0f, .0f,  .0f, .0f, // bottom 2

    -.5f, .5f,  .5f,  .0f,  1.0f,  .0f,  .0f, .0f, // top
    .5f,  .5f,  .5f,  .0f,  1.0f,  .0f,  1.f, .0f, // top
    .5f,  .5f,  -.5f, .0f,  1.0f,  .0f,  1.f, 1.f, // top
    .5f,  .5f,  -.5f, .0f,  1.0f,  .0f,  1.f, 1.f, // top 2
    -.5f, .5f,  -.5f, .0f,  1.0f,  .0f,  .0f, 1.f, // top 2
    -.5f, .5f,  .5f,  .0f,  1.0f,  .0f,  .0f, .0f, // top 2

};

glm::vec3 cube_positions[] = {
    glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
    glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

light_t light = {.position = glm::vec3(2.f, 1.f, -2.f),
                 .ambient = glm::vec3(.2f, .2f, .2f),
                 .diffuse = glm::vec3(.5f, .5f, .5f),
                 .specular = glm::vec3(1.f, 1.f, 1.f)};

struct buffers_t {
  unsigned int cube_vao;
  unsigned int light_vao;

  cleanup_t cleanup = []() {};
};

buffers_t buffers();
unsigned int create_texture_empty();
void strobe_light(light_t &light, double now);

std::expected<hooks_t, std::string> init_shaders() {
  auto shader = Shader::build("shaders/14_view_strobe.vert",
                              "shaders/14_view_strobe.frag");
  if (!shader) {
    return std::unexpected(shader.error());
  }
  auto light_shader = Shader::build("shaders/14_light_strobe.vert",
                                    "shaders/14_light_strobe.frag");
  if (!light_shader) {
    return std::unexpected(light_shader.error());
  }
  auto p = shader->ID;
  auto light_id = light_shader->ID;
  const std::string filename{"textures/container.jpg"};
  auto texture_ = load_texture(filename);
  if (!texture_) {
    return std::unexpected(texture_.error());
  }
  auto texture = *texture_;
  texture_ = load_texture("textures/awesomeface.png");
  if (!texture_) {
    return std::unexpected(texture_.error());
  }
  auto texture1 = *texture_;
  auto texture_empty = create_texture_empty();
  auto vaos = buffers();
  auto cube_vao = vaos.cube_vao;
  auto light_vao = vaos.light_vao;

  auto f = [p, light_id, cube_vao, light_vao, texture, texture1,
            texture_empty](uint64_t e, float delta) {
    auto now = glfwGetTime();
    strobe_light(light, now);
    material_t material = materials[current_material];

    if (e & event_t::increase_fov) {
      camera.update_fov(1.f);
    }
    if (e & event_t::decrease_fov) {
      camera.update_fov(-1.f);
    }
    if (e & event_t::camera_up) {
      camera.process_movement(CameraMovement::UP, delta);
    }
    if (e & event_t::camera_down) {
      camera.process_movement(CameraMovement::DOWN, delta);
    }
    if (e & event_t::camera_left) {
      camera.process_movement(CameraMovement::LEFT, delta);
    }
    if (e & event_t::camera_right) {
      camera.process_movement(CameraMovement::RIGHT, delta);
    }
    if (e & event_t::camera_for) {
      camera.process_movement(CameraMovement::FORWARD, delta);
    }
    if (e & event_t::camera_back) {
      camera.process_movement(CameraMovement::BACKWARD, delta);
    }
    if (e & event_t::camera_yaw_left) {
      camera.process_rotation(120 * -SPEED * delta, 0.f);
    }
    if (e & event_t::camera_yaw_right) {
      camera.process_rotation(120 * SPEED * delta, 0.f);
    }
    if (e & event_t::light_up) {
      light.position += glm::vec3(0.f, 0.f, SPEED * delta);
    }
    if (e & event_t::light_down) {
      light.position -= glm::vec3(0.f, 0.f, SPEED * delta);
    }
    if (e & event_t::light_up) {
      light.position += glm::vec3(0.f, SPEED * delta, 0.f);
    }
    if (e & event_t::light_down) {
      light.position -= glm::vec3(0.f, SPEED * delta, 0.f);
    }
    if (e & event_t::light_left) {
      light.position -= glm::vec3(SPEED * delta, 0.f, 0.f);
    }
    if (e & event_t::light_right) {
      light.position += glm::vec3(SPEED * delta, 0.f, 0.f);
    }
    if (e & event_t::light_for) {
      light.position -= glm::vec3(0.f, 0.f, SPEED * delta);
    }
    if (e & event_t::light_back) {
      light.position += glm::vec3(0.f, 0.f, SPEED * delta);
    }
    glUseProgram(p);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures_enabled ? texture : texture_empty);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures_enabled ? texture1 : texture_empty);

    glm::mat4 model;

    glm::mat4 view = camera.get_view_matrix();

    glm::mat4 projection =
        glm::perspective(glm::radians(camera.fov),
                         static_cast<float>(viewport.width) /
                             static_cast<float>(viewport.height),
                         .1f, 100.f);

    set_mat4(p, "view", view);
    set_mat4(p, "projection", projection);
    set_vec3(p, "light_pos", light.position);
    set_vec3(p, "view_pos", camera.position);
    set_material(p, "material", material);
    set_light(p, "light", light);

    glBindVertexArray(cube_vao);

    glUseProgram(light_id);

    set_mat4(light_id, "view", view);
    set_mat4(light_id, "projection", projection);
    set_light(light_id, "light", light);

    float angle;
    for (unsigned int i = 0; i < 10; ++i) {
      angle = 20.f * i;
      angle = glfwGetTime() * (i % 3) * 25.f;
      model = glm::translate(glm::mat4(1.f), cube_positions[i]);
      model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));

      glUseProgram(p);
      set_mat4(p, "model", model);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    model = glm::mat4(1.f);
    model = glm::translate(model, light.position);
    model = glm::scale(model, glm::vec3(.2f));

    glUseProgram(light_id);
    set_mat4(light_id, "model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
  };

  auto imgui = [](uint64_t e, float delta) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(6.0f, 6.0f), ImGuiCond_Once);
    ImGui::Begin("Scene information", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(150.0f);

    ImGui::LabelText("Pos", "(%.2f, %.2f, %.2f)", camera.position.x,
                     camera.position.y, camera.position.z);

    ImGui::PopItemWidth();
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  };

  shader->use();
  shader->set_int("texture1", 0);
  shader->set_int("texture2", 1);

  cbs_t v{f, imgui};
  return hooks_t{.callbacks = v, .cleanup = vaos.cleanup};
}

void strobe_light(light_t &light, double now) {
  glm::vec3 color{sin(now) * 2.f, sin(now) * .7f, cos(now) * 1.3f};
  light.diffuse = color * glm::vec3(.5f);
  light.ambient = light.diffuse * glm::vec3(.2f);
}

buffers_t buffers() {
  unsigned int cube_vao;
  unsigned int vbo;
  glGenVertexArrays(1, &cube_vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(cube_vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * (sizeof(float))));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * (sizeof(float))));
  glEnableVertexAttribArray(2);

  unsigned int light_vao;
  glGenVertexArrays(1, &light_vao);
  glBindVertexArray(light_vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  auto cleanup = [cube_vao, light_vao, vbo]() {
    glDeleteVertexArrays(1, &cube_vao);
    glDeleteVertexArrays(1, &light_vao);
    glDeleteBuffers(1, &vbo);
  };

  return {.cube_vao = cube_vao, .light_vao = light_vao, .cleanup = cleanup};
}

unsigned int create_texture_empty() {
  unsigned int texture;
  uint8_t white[] = {255, 255, 255, 255};
  auto width = 1, height = 1;
  auto format = GL_RGB;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format,
               GL_UNSIGNED_BYTE, white);
  return texture;
}

void process_input(GLFWwindow *window, uint64_t &e);

struct delta_t {
  float last;  // Time of last frame
  float delta; // Time between current frame and last frame
};

void update_delta(delta_t &delta) {
  float now = glfwGetTime();
  delta.delta = now - delta.last;
  delta.last = now;
}

void event_loop(GLFWwindow *window,
                std::vector<std::function<void(uint64_t, float)>> cbs) {
  uint64_t e;

  delta_t delta{};

  while (!glfwWindowShouldClose(window)) {
    update_delta(delta);

    e = event_t::NONE;
    process_input(window, e);
    glClearColor(.2f, .3f, .3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto cb : cbs) {
      cb(e, delta.delta);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void process_input(GLFWwindow *window, uint64_t &e) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    e |= event_t::increase_fov;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    e |= event_t::decrease_fov;
  }
  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
    e |= event_t::camera_up;
  }
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
    e |= event_t::camera_down;
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    e |= event_t::camera_left;
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    e |= event_t::camera_right;
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    e |= event_t::camera_for;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    e |= event_t::camera_back;
  }
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    e |= event_t::camera_yaw_left;
  }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    e |= event_t::camera_yaw_right;
  }
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    e |= event_t::light_for;
  }
  if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
    e |= event_t::light_back;
  }
  if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
    e |= event_t::light_left;
  }
  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    e |= event_t::light_right;
  }
  if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
    e |= event_t::light_up;
  }
  if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
    e |= event_t::light_down;
  }
}

void material_next();
void material_prev();

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_M && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    if (mods == GLFW_MOD_SHIFT) {
      material_prev();
    } else {
      material_next();
    }
  }
  if (key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    textures_enabled ^= true;
  }
}

const float MOUSE_SENSITIVITY = .1f;

void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
  ImGuiIO &io = ImGui::GetIO();
  if (io.WantCaptureMouse) {
    return;
  }

  static float x = viewport.width / 2;
  static float y = viewport.height / 2;

  if (mouse_new_focus) {
    x = x_pos;
    y = y_pos;
    mouse_new_focus = false;
  }

  float x_offset = x_pos - x;
  float y_offset = y - y_pos;
  x = x_pos;
  y = y_pos;
  camera.process_rotation(x_offset, y_offset);
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset) {
  camera.update_fov(static_cast<float>(y_offset));
}

void material_next() {
  current_material = (current_material + 1) % materials.size();
}
void material_prev() {
  current_material =
      (current_material - 1 + materials.size()) % materials.size();
}
