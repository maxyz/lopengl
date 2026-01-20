#include <iostream>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;

struct vaos {
  unsigned int a;
  unsigned int b;
};

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int shadersCode();
struct vaos buffers();

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  int version = gladLoadGL(glfwGetProcAddress);
  std::cout << "GL " << GLAD_VERSION_MAJOR(version) << "."
            << GLAD_VERSION_MINOR(version) << std::endl;

  glViewport(0, 0, WIDTH, HEIGHT);
  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  auto shaderProgram = shadersCode();
  auto vaos = buffers();

  float delta = 0.001, r = 0.2, g = 0.3, b = 0.3;

  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    glClearColor(r, g, b, 1.0f);
    r = (r + delta);
    r = r > 1.0f ? r - 1.0f : r;
    g = (g + delta);
    g = g > 1.0f ? g - 1.0f : g;
    b = (b + delta);
    b = b > 1.0f ? b - 1.0f : b;
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(vaos.a);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(vaos.b);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}
float h = 0.5773502691896257f;
float vertices_a[] = {
    -0.666666f, -h / 2, 0.0f, // bottom left
    0.0f,       -h / 2, 0.0f, // bottom center
    -0.333333f, h / 2,  0.0f, // top left
};

float vertices_b[] = {
    0.0f,      -h / 2, 0.0f, // bottom center
    0.333333f, h / 2,  0.0f, // top right
    0.666666,  -h / 2, 0.0f, // bottom right
};

const char *vertexShaderSource =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "  FragColor = vec4(1.0f, 0.5f, 0.2, 1.0f);\n"
    "}\0";

unsigned int shadersCode() {
  int success;
  char infoLog[512];

  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  unsigned int fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
              << infoLog << std::endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

struct vaos buffers() {
  unsigned int VAO;
  unsigned int VAOB;
  unsigned int VBO;

  glGenVertexArrays(1, &VAO);
  glGenVertexArrays(1, &VAOB);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_a), vertices_a, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glGenBuffers(1, &VBO);

  glBindVertexArray(VAOB);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_b), vertices_b, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  return {VAO, VAOB};
}
