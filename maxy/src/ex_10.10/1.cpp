#include <expected>
#include <functional>
#include <glm/geometric.hpp>
#include <iostream>
#include <vector>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "common/assets.hpp"
#include "common/camera.hpp"
#include "common/geometry.hpp"
#include "common/shader.hpp"

const char *TITLE = "LOpenGL";
const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;

struct view_t {
  float width = WIDTH;
  float height = HEIGHT;
};
view_t viewport;

Camera camera = Camera(glm::vec3(0.f, 0.f, 3.f));

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
};

using cb_t = std::function<void(uint64_t, float)>;
using cbs_t = std::vector<cb_t>;

std::expected<GLFWwindow *, std::string> init_window();
std::expected<cbs_t, std::string> init_shaders();
std::expected<void, std::string> init_textures();

void event_loop(GLFWwindow *window, cbs_t cbs);

int main() {
  camera.fly(false);
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
  event_loop(*window, *res);

  glfwTerminate();
  return 0;
}

void mouse_callback(GLFWwindow *window, double x_pos, double y_pos);
void scroll_callback(GLFWwindow *window, double x_offset, double y_offset);

void framebufferSizeCallback(GLFWwindow *window, int width, int height);

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
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_DEPTH_TEST);
  return window;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  viewport.width = width;
  viewport.height = height;
  glViewport(0, 0, width, height);
}

const float vertices[] = {
    -.5f, -.5f, -.5f, .0f, .0f, // back
    .5f,  -.5f, -.5f, 1.f, .0f, // back
    .5f,  .5f,  -.5f, 1.f, 1.f, // back
    .5f,  .5f,  -.5f, 1.f, 1.f, // back 2
    -.5f, .5f,  -.5f, .0f, 1.f, // back 2
    -.5f, -.5f, -.5f, .0f, .0f, // back 2

    .5f,  -.5f, .5f,  .0f, .0f, // front
    -.5f, -.5f, .5f,  1.f, .0f, // front
    -.5f, .5f,  .5f,  1.f, 1.f, // front
    -.5f, .5f,  .5f,  1.f, 1.f, // front 2
    .5f,  .5f,  .5f,  .0f, 1.f, // front 2
    .5f,  -.5f, .5f,  .0f, .0f, // front 2

    -.5f, -.5f, .5f,  .0f, .0f, // left
    -.5f, -.5f, -.5f, 1.f, .0f, // left
    -.5f, .5f,  -.5f, 1.f, 1.f, // left
    -.5f, .5f,  -.5f, 1.f, 1.f, // left 2
    -.5f, .5f,  .5f,  .0f, 1.f, // left 2
    -.5f, -.5f, .5f,  .0f, .0f, // left 2

    .5f,  -.5f, -.5f, .0f, .0f, // right
    .5f,  -.5f, .5f,  1.f, .0f, // right
    .5f,  .5f,  .5f,  1.f, 1.f, // right
    .5f,  .5f,  .5f,  1.f, 1.f, // right 2
    .5f,  .5f,  -.5f, .0f, 1.f, // right 2
    .5f,  -.5f, -.5f, .0f, .0f, // right 2

    -.5f, -.5f, -.5f, .0f, .0f, // bottom
    .5f,  -.5f, -.5f, 1.f, .0f, // bottom
    .5f,  -.5f, .5f,  1.f, 1.f, // bottom
    .5f,  -.5f, .5f,  1.f, 1.f, // bottom 2
    -.5f, -.5f, .5f,  .0f, 1.f, // bottom 2
    -.5f, -.5f, -.5f, .0f, .0f, // bottom 2

    -.5f, .5f,  .5f,  .0f, .0f, // top
    .5f,  .5f,  .5f,  1.f, .0f, // top
    .5f,  .5f,  -.5f, 1.f, 1.f, // top
    .5f,  .5f,  -.5f, 1.f, 1.f, // top 2
    -.5f, .5f,  -.5f, .0f, 1.f, // top 2
    -.5f, .5f,  .5f,  .0f, .0f, // top 2

};

unsigned int buffers();
void update_camera_front();

std::expected<cbs_t, std::string> init_shaders() {
  auto res = Shader::build("shaders/10_camera_circle.vert",
                           "shaders/10_camera_circle.frag");
  if (!res.has_value()) {
    return std::unexpected(res.error());
  }
  auto shader = *res;
  auto p = shader.ID;
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
  auto vao = buffers();

  auto f = [p, vao, texture, texture1](uint64_t e, float delta) {
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
      camera.process_rotation(-1.f, delta);
    }
    if (e & event_t::camera_yaw_right) {
      camera.process_rotation(1.f, delta);
    }
    glUseProgram(p);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glm::mat4 model;

    glm::mat4 view = camera.get_view_matrix();

    glm::mat4 projection =
        glm::perspective(glm::radians(camera.fov),
                         static_cast<float>(viewport.width) /
                             static_cast<float>(viewport.height),
                         .1f, 100.f);

    GLint loc;

    loc = glGetUniformLocation(p, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));

    loc = glGetUniformLocation(p, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    float angle;

    for (unsigned int i = 0; i < 10; ++i) {
      angle = 20.f * i;
      if (i % 3 == 0)
        angle = glfwGetTime() * 25.f;
      model = glm::translate(glm::mat4(1.f), example_cube_positions[i]);
      model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));
      loc = glGetUniformLocation(p, "model");
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  };

  shader.use();
  shader.set_int("texture1", 0);
  shader.set_int("texture2", 1);

  cbs_t v{f};
  return v;
}

unsigned int buffers() {
  unsigned int VAO;
  unsigned int VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void *>(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

  return VAO;
}

void process_input(GLFWwindow *window, uint64_t &e);

struct frame_time_t {
  float last_time; // Time of last frame
  float delta;     // Time between current frame and last frame
};

void update_delta(frame_time_t &delta) {
  float now = glfwGetTime();
  delta.delta = now - delta.last_time;
  delta.last_time = now;
}

void event_loop(GLFWwindow *window,
                std::vector<std::function<void(uint64_t, float)>> cbs) {
  uint64_t e;

  frame_time_t delta{};

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
}

const float MOUSE_SENSITIVITY = .1f;

void mouse_callback(GLFWwindow *window, double x_pos, double y_pos) {
  static bool first_mouse = true;
  static float x = viewport.width / 2;
  static float y = viewport.height / 2;

  if (first_mouse) {
    x = x_pos;
    y = y_pos;
    first_mouse = false;
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
