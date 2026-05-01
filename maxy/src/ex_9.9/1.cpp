#include <cmath>
#include <expected>
#include <functional>
#include <iostream>
#include <vector>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "common/assets.hpp"
#include "common/shader.hpp"

const char *TITLE = "LOpenGL";
const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;

enum event_t {
  NONE = 0,
  increase_fov = 1 << 0,
  decrease_fov = 1 << 1,
};

std::expected<GLFWwindow *, std::string> init_window();
std::expected<std::vector<std::function<void(uint64_t)>>, std::string>
init_shaders();
std::expected<void, std::string> init_textures();

void event_loop(GLFWwindow *window,
                std::vector<std::function<void(uint64_t)>> cbs);

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
  event_loop(*window, *res);

  glfwTerminate();
  return 0;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height);

std::expected<GLFWwindow *, std::string> init_window() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);
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
  glViewport(0, 0, WIDTH, HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glEnable(GL_DEPTH_TEST);

  return window;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
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

glm::vec3 cube_positions[] = {
    glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
    glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

unsigned int buffers();

float fov = 45.f;

std::expected<std::vector<std::function<void(uint64_t)>>, std::string>
init_shaders() {
  auto res =
      Shader::build("shaders/9_going_3d.vert", "shaders/9_going_3d.frag");
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

  auto f = [p, vao, texture, texture1](uint64_t e) {
    if (e & event_t::increase_fov) {
      fov += 1.f;
    }
    if (e & event_t::decrease_fov) {
      fov -= 1.f;
    }
    glUseProgram(p);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glm::mat4 model;

    glm::mat4 view = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -3.f));

    glm::mat4 projection = glm::perspective(
        glm::radians(fov),
        static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), .1f, 100.f);

    GLint loc;

    loc = glGetUniformLocation(p, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));

    loc = glGetUniformLocation(p, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    float angle;

    for (unsigned int i = 0; i < 10; ++i) {
      angle = 20.f * i;
      model = glm::translate(glm::mat4(1.f), cube_positions[i]);
      model = glm::rotate(model, glm::radians(angle), glm::vec3(1.f, .3f, .5f));
      loc = glGetUniformLocation(p, "model");
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
  };

  shader.use();
  shader.set_int("texture1", 0);
  shader.set_int("texture2", 1);

  std::vector<std::function<void(uint64_t)>> v{f};
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

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * (sizeof(float))));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

  return VAO;
}

void process_input(GLFWwindow *window, uint64_t &e);

void event_loop(GLFWwindow *window,
                std::vector<std::function<void(uint64_t)>> cbs) {
  uint64_t e;
  while (!glfwWindowShouldClose(window)) {
    e = event_t::NONE;
    process_input(window, e);
    glClearColor(.2f, .3f, .3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto cb : cbs) {
      cb(e);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void process_input(GLFWwindow *window, uint64_t &e) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  } else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    e |= event_t::increase_fov;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    e |= event_t::decrease_fov;
  }
}
