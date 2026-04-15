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

enum event_t {
  NONE = 0,
  ML = 1 << 0,
  MR = 1 << 1,
  MU = 1 << 2,
  MD = 1 << 3,
};

using cb_t = std::function<void(event_t)>;

std::expected<GLFWwindow *, std::string> init_window();
std::expected<std::vector<cb_t>, std::string> init_shaders();

void event_loop(GLFWwindow *window, std::vector<cb_t> cbs);

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
    .5f,  .5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.f, 1.f, // top right
    -.5f, .5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.f, 1.f, // top left
    .5f,  -.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.f, 0.f, // bottom right
    -.5f, -.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.f, 0.f, // bottom left
};
const unsigned int indices[] = {
    0, 2, 3, // first triangle
    0, 1, 3, // second triangle
};

unsigned int buffers();
std::expected<unsigned int, std::string>
load_texture(const std::string &filename, GLenum format = GL_RGB,
             GLenum wrap = GL_REPEAT);

std::expected<std::vector<cb_t>, std::string> init_shaders() {
  auto res = Shader::build("shaders/ex_7.8.4.vert", "shaders/ex_7.8.4.frag");
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
  texture_ = load_texture("textures/awesomeface.png", GL_RGBA, GL_REPEAT);
  if (!texture_) {
    return std::unexpected(texture_.error());
  }
  auto texture1 = *texture_;
  auto vao = buffers();

  auto update_level = [p](event_t e) {
    GLfloat delta = 0.;
    switch (e) {
    case (event_t::MU):
      delta = 0.01f;
      break;
    case (event_t::MD):
      delta = -0.01f;
      break;
    default:
      return;
    }
    auto levelLocation = glGetUniformLocation(p, "level");
    GLfloat level;
    glGetUniformfv(p, levelLocation, &level);
    level = std::clamp(level + delta, 0.f, 1.f);
    glUseProgram(p);
    glUniform1fv(levelLocation, 1, &level);
  };

  auto f = [p, vao, texture, texture1](event_t) {
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

  std::vector<cb_t> v{update_level, f};
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

std::expected<unsigned int, std::string>
load_texture(const std::string &filename, GLenum format, GLenum wrap) {
  auto image = load_image(filename);
  if (!image) {
    return std::unexpected(image.error());
  }

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  float borderColor[] = {1.0f, 1.0f, 0.0f, 1.0f};

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, format,
               GL_UNSIGNED_BYTE, image->data.get());
  glGenerateMipmap(GL_TEXTURE_2D);

  return texture;
}

void processInput(GLFWwindow *window, event_t &e);

void event_loop(GLFWwindow *window, std::vector<cb_t> cbs) {
  event_t e = event_t::NONE;
  while (!glfwWindowShouldClose(window)) {
    e = event_t::NONE;
    processInput(window, e);
    glClearColor(0.2, 0.3, 0.3, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto cb : cbs) {
      cb(e);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void processInput(GLFWwindow *window, event_t &e) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  } else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  } else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    e = event_t::ML;
  } else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    e = event_t::MR;
  } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    e = event_t::MU;
  } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    e = event_t::MD;
  }
}
