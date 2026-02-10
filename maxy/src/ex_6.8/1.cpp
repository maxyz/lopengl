#include <cmath>
#include <expected>
#include <functional>
#include <iostream>
#include <vector>

#include <glad/gl.h>

#include "common/shader.hpp"
#include <GLFW/glfw3.h>

const char *TITLE = "LOpenGL";
const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;

std::expected<GLFWwindow *, std::string> init_window();
std::expected<std::vector<std::function<void()>>, std::string> init_shaders();
void event_loop(GLFWwindow *window, std::vector<std::function<void()>> cbs);

int main() {
  // std::cerr << "init_window" << std::endl;
  auto window = init_window();
  if (!window.has_value()) {
    std::cerr << window.error() << std::endl;
    return -1;
  }

  // std::cerr << "init_shaders" << std::endl;
  auto res = init_shaders();
  if (!res.has_value()) {
    std::cerr << res.error() << std::endl;
    return -1;
  }
  auto cbs = res.value();

  // std::cerr << "event_loop" << std::endl;
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
    return std::unexpected("failed to init glad on top of glfw");
  }
  glViewport(0, 0, WIDTH, HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

  return window;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

float h = std::sin(M_PI / 3);

float vertices[] = {
    .5f,   -h / 2, 0.0f, 1.0f, 0.0f, 0.0f, // bottom right
    -0.5f, -h / 2, 0.0f, 0.0f, 1.0f, 0.0f, // bottom left
    0.0f,  h / 2,  0.0f, 0.0f, 0.0f, 1.0f, // top
};

std::expected<unsigned int, std::string> compile_shader(const GLenum type,
                                                        const char *source);
std::expected<unsigned int, std::string>
link_shaders(std::vector<unsigned int> shaders);
unsigned int buffers();

std::expected<std::vector<std::function<void()>>, std::string> init_shaders() {
  auto res = Shader::build("shaders/1.vert", "shaders/1.frag");
  if (!res.has_value()) {
    return std::unexpected(res.error());
  }
  auto shader = *res;
  auto p = shader.ID;
  auto vao = buffers();

  auto f = [p, vao]() {
    glUseProgram(p);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  };

  std::vector<std::function<void()>> v{f};
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

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                        (void *)(3 * (sizeof(float))));
  glEnableVertexAttribArray(1);

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
