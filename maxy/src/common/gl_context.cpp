#include "common/gl_context.hpp"
#include "common/assets.hpp"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/gl.h>
#include <imgui.h>

static std::expected<GLFWwindow *, std::string> init_glfw(int width, int height,
                                                          const char *title) {
  glfwInit();
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

std::expected<GLContext, std::string> GLContext::create(int width, int height,
                                                        const char *title) {
  return init_glfw(width, height, title)
      .and_then([](GLFWwindow *w) {
        return init_imgui(w)
            .transform_error([](std::string e) {
              glfwTerminate();
              return e;
            })
            .transform([w] { return w; });
      })
      .transform([](GLFWwindow *w) {
        GLContext ctx;
        ctx.m_window = w;
        return ctx;
      });
}

GLContext::~GLContext() {
  if (!m_window)
    return;
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
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
