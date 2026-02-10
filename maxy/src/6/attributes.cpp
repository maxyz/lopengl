#include <cmath>
#include <expected>
#include <functional>
#include <iostream>
#include <vector>

#include <glad/gl.h>

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
  auto cbs = init_shaders();
  if (!cbs.has_value()) {
    std::cerr << cbs.error() << std::endl;
    return -1;
  }

  // std::cerr << "event_loop" << std::endl;
  event_loop(window.value(), cbs.value());

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

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec3 aColor;\n"
                                 "out vec3 ourColor;\n"
                                 "void main()\n"
                                 "{\n"
                                 "  gl_Position = vec4(aPos, 1.0);\n"
                                 "  ourColor = aColor;\n"
                                 "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec3 ourColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "  FragColor = vec4(ourColor, 1.0);\n"
                                   "}\0";

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
  // std::cerr << "compile_shader" << std::endl;
  auto vertex = compile_shader(GL_VERTEX_SHADER, vertexShaderSource);
  if (!vertex.has_value()) {
    return std::unexpected(vertex.error());
  }
  // std::cerr << vertex.value() << std::endl;

  // std::cerr << "compile_shader fragment" << std::endl;
  auto fragment = compile_shader(GL_FRAGMENT_SHADER, fragmentShaderSource);
  if (!fragment.has_value()) {
    return std::unexpected(fragment.error());
  }
  // std::cerr << fragment.value() << std::endl;

  std::vector<unsigned int> shaders{vertex.value(), fragment.value()};
  // std::cerr << "link_shaders" << std::endl;
  auto program = link_shaders(shaders);
  if (!program.has_value()) {
    return std::unexpected(program.error());
  }

  // std::cerr << "deletes" << std::endl;
  glDeleteShader(vertex.value());
  glDeleteShader(fragment.value());

  auto p = program.value();
  auto vao = buffers();

  auto f = [p, vao]() {
    glUseProgram(p);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
  };

  std::vector<std::function<void()>> res{f};
  return res;
}

const std::string_view type_to_view(const GLenum type);

std::expected<unsigned int, std::string> compile_shader(const GLenum type,
                                                        const char *source) {
  int success;
  char info_log[512];

  unsigned int shader;
  shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, info_log);
    auto error = std::format("error shader {}, compilation failed\n{}",
                             type_to_view(type), info_log);
    return std::unexpected(error);
  }
  return shader;
}

std::expected<unsigned int, std::string>
link_shaders(std::vector<unsigned int> shaders) {
  int success;
  char info_log[512];

  unsigned int program;
  // std::cerr << "glCreateProgram" << std::endl;
  program = glCreateProgram();
  for (auto shader : shaders) {
    // std::cerr << "glAttachShader " << shader << std::endl;
    glAttachShader(program, shader);
  }
  glLinkProgram(program);
  // std::cerr << "glGetProgramiv" << std::endl;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    // std::cerr << "glGetProgramInfoLog" << std::endl;
    glGetProgramInfoLog(program, 512, NULL, info_log);
    // std::cerr << "format" << std::endl;
    auto error = std::format("error shader link failed\n{}", info_log);
    return std::unexpected(error);
  }
  return program;
}

const std::string_view type_to_view(const GLenum type) {
  switch (type) {
  case (GL_VERTEX_SHADER):
    return "vertex";
  case (GL_FRAGMENT_SHADER):
    return "fragment";
  default:
    return "unknown";
  }
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
