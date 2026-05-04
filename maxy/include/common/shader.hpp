#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <expected>
#include <span>
#include <string>
#include <utility>

#include "common/types.hpp"

class Shader {
public:
  Shader() = default;
  explicit Shader(id_t program_id) : m_program_id(program_id) {}
  ~Shader() { glDeleteProgram(m_program_id); }

  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;
  Shader(Shader &&o) noexcept : m_program_id(std::exchange(o.m_program_id, 0)) {}
  Shader &operator=(Shader &&o) noexcept {
    if (this != &o) {
      glDeleteProgram(m_program_id);
      m_program_id = std::exchange(o.m_program_id, 0);
    }
    return *this;
  }

  using build_res = std::expected<Shader, std::string>;
  static build_res build(std::string_view vertexPath,
                         std::string_view fragmentPath);

  id_t program_id() const { return m_program_id; }

  void use();
  void set_bool(const std::string &name, bool value) const;
  void set_int(const std::string &name, int value) const;
  void set_float(const std::string &name, float value) const;
  void set_vec3(const std::string &name, const glm::vec3 &value) const;
  void set_vec4(const std::string &name, const glm::vec4 &value) const;
  void set_mat4(const std::string &name, const glm::mat4 &value) const;

private:
  id_t m_program_id{};
};

using compile_shader_res = std::expected<id_t, std::string>;
compile_shader_res compile_shader(GLenum type, const char *source);

using link_shaders_res = std::expected<id_t, std::string>;
link_shaders_res link_shaders(std::span<const id_t> shaders);

void set_int(id_t id, const std::string &name, int value);
void set_float(id_t id, const std::string &name, float value);
void set_vec3(id_t id, const std::string &name, const glm::vec3 &value);
void set_vec4(id_t id, const std::string &name, const glm::vec4 &value);
void set_mat4(id_t id, const std::string &name, const glm::mat4 &value);
