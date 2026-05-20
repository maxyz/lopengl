#include <format>

#include "common/assets.hpp"
#include "common/gl_context.hpp"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <imgui.h>
#include <stb/stb_image.h>

static std::expected<GLFWwindow *, std::string>
init_glfw(int width, int height, const char *title) {
  if (glfwInit() == GLFW_FALSE) {
    char *error_description;
    glfwGetError((const char **)&error_description);
    return std::unexpected(
        std::format("failed to init GLFW: {}", error_description)
    );
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return std::unexpected("failed to create GLFW window");
  }
  glfwMakeContextCurrent(window);
  if (gladLoadGL(glfwGetProcAddress) == 0) {
    glfwTerminate();
    return std::unexpected("failed to init glad");
  }
  glViewport(0, 0, width, height);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_STENCIL_TEST);

  stbi_set_flip_vertically_on_load(true);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  return window;
}

static std::expected<void, std::string> init_imgui(GLFWwindow *w) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  auto font_path = get_asset_path("fonts/NotoSans-Regular.ttf");
  if (!font_path) {
    return std::unexpected("failed to obtain default font");
  }
  io.Fonts->AddFontFromFileTTF(font_path->c_str(), 20);
  io.IniFilename = nullptr;
  ImGui::StyleColorsClassic();
  ImGui_ImplGlfw_InitForOpenGL(w, true);
  ImGui_ImplOpenGL3_Init();
  return {};
}

std::expected<GLContext, std::string>
GLContext::create(int width, int height, const char *title) {
  auto window = init_glfw(width, height, title);
  if (!window) {
    return std::unexpected(window.error());
  }
  auto imgui_res = init_imgui(*window);
  if (!imgui_res) {
    glfwTerminate();
    return std::unexpected(imgui_res.error());
  }
  return GLContext{*window};
}

GLContext::~GLContext() {
  if (!m_window)
    return;
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

GLContext::GLContext(GLContext &&other) noexcept : m_window(other.m_window) {
  other.m_window = nullptr;
}

GLContext &GLContext::operator=(GLContext &&other) noexcept {
  if (this != &other) {
    if (m_window) {
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplGlfw_Shutdown();
      ImGui::DestroyContext();
      glfwTerminate();
    }
    m_window = other.m_window;
    other.m_window = nullptr;
  }
  return *this;
}
