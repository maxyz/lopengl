#include <cmath>
#include <expected>
#include <functional>
#include <iostream>
#include <vector>

#include <glad/gl.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "common/assets.hpp"
#include "common/shader.hpp"

const char *TITLE = "LOpenGL";
const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;

std::expected<GLFWwindow *, std::string> init_window();
std::expected<std::vector<std::function<void()>>, std::string> init_shaders();
std::expected<void, std::string> init_textures();

void event_loop(GLFWwindow *window, std::vector<std::function<void()>> cbs);

int main() {
  auto window = init_window();
  if (!window.has_value()) {
    std::cerr << window.error() << std::endl;
    return -1;
  }

  auto res = init_shaders();
  if (!res.has_value()) {
    std::cerr << res.error() << std::endl;
    return -1;
  }
  auto cbs = res.value();

  event_loop(window.value(), cbs);

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

  return window;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

const float h = std::sin(M_PI / 3);

const float vertices[] = {
    .5f,  .5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
    -.5f, .5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top left
    .5f,  -.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
    -.5f, -.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
};
const unsigned int indices[] = {
    0, 2, 3, // first triangle
    0, 1, 3, // second triangle
};

unsigned int buffers();

std::expected<std::vector<std::function<void()>>, std::string> init_shaders() {
  auto res =
      Shader::build("shaders/7_combined.vert", "shaders/7_combined.frag");
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

  auto f = [p, vao, texture, texture1]() {
    glUseProgram(p);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  };

  shader.use();
  shader.set_int("texture1", 0);
  shader.set_int("texture2", 1);

  std::vector<std::function<void()>> v{f};
  return v;
}

unsigned int buffers() {
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * (sizeof(float))));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * (sizeof(float))));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return VAO;
}

void processInput(GLFWwindow *window);

void event_loop(GLFWwindow *window, std::vector<std::function<void()>> cbs) {
  while (!glfwWindowShouldClose(window)) {
    processInput(window);
    glClearColor(0.2, 0.3, 0.3, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto cb : cbs) {
      cb();
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  } else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}
