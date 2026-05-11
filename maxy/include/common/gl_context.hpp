#pragma once

#include <expected>
#include <string>

struct GLFWwindow;

class GLContext {
public:
  static std::expected<GLContext, std::string> create(int width, int height,
                                                      const char *title);

  ~GLContext();
  GLContext(const GLContext &) = delete;
  GLContext &operator=(const GLContext &) = delete;
  GLContext(GLContext &&) noexcept;
  GLContext &operator=(GLContext &&) noexcept;

  GLFWwindow *window() const { return m_window; }

private:
  GLFWwindow *m_window = nullptr;

  GLContext(GLFWwindow *window) : m_window(window) {}
};
