#include <iostream>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

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
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}
