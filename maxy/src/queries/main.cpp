#include <expected>
#include <iostream>

#include <glad/gl.h>

#include <GLFW/glfw3.h>

const char *TITLE = "LOpenGL";
const GLuint WIDTH = 800;
const GLuint HEIGHT = 600;

std::expected<GLFWwindow *, std::string> init_window();
void query();

int main() {
  auto window = init_window();
  if (!window.has_value()) {
    std::cerr << window.error() << std::endl;
    return -1;
  }

  query();

  glfwTerminate();
  return 0;
}

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

  return window;
}

void query() {
  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes
            << std::endl;
}
