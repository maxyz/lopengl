#include "common/shader.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

using read_file_res = std::expected<std::string, std::string>;
read_file_res read_file(const std::filesystem::path &path);

static compile_shader_res compile_file(std::string_view path, GLenum type) {
  return read_file(std::filesystem::path(path))
      .and_then([type](const std::string &code) {
        return compile_shader(type, code.c_str());
      })
      .transform_error([path](std::string e) {
        return std::format("error compiling {}\n{}", path, e);
      });
}

static std::expected<std::pair<id_t, id_t>, std::string>
with_fragment(id_t v_shader, std::string_view fragmentPath) {
  return compile_file(fragmentPath, GL_FRAGMENT_SHADER)
      .transform([v_shader](id_t f_shader) {
        return std::pair{v_shader, f_shader};
      });
}

static Shader::build_res link_and_clean(std::pair<id_t, id_t> shaders) {
  auto [v, f] = shaders;
  return link_shaders({v, f}).transform([v, f](id_t program) {
    glDeleteShader(v);
    glDeleteShader(f);
    return Shader{program};
  });
}

Shader::build_res Shader::build(std::string_view vertexPath,
                                std::string_view fragmentPath) {
  return compile_file(vertexPath, GL_VERTEX_SHADER)
      .and_then([fragmentPath](id_t v) { return with_fragment(v, fragmentPath); })
      .and_then(link_and_clean);
}

void Shader::use() { glUseProgram(m_program_id); }

void Shader::set_bool(const std::string &name, bool value) const {
  glUniform1i(glGetUniformLocation(m_program_id, name.c_str()),
              static_cast<int>(value));
}
void Shader::set_int(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(m_program_id, name.c_str()), value);
}
void Shader::set_float(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(m_program_id, name.c_str()), value);
}
void Shader::set_vec3(const std::string &name, const glm::vec3 &value) const {
  glUniform3fv(glGetUniformLocation(m_program_id, name.c_str()), 1,
               glm::value_ptr(value));
}
void Shader::set_vec4(const std::string &name, const glm::vec4 &value) const {
  glUniform4fv(glGetUniformLocation(m_program_id, name.c_str()), 1,
               glm::value_ptr(value));
}
void Shader::set_mat4(const std::string &name, const glm::mat4 &value) const {
  glUniformMatrix4fv(glGetUniformLocation(m_program_id, name.c_str()), 1,
                     GL_FALSE, glm::value_ptr(value));
}

const std::string_view type_to_view(const GLenum type);

constexpr int shader_info_log_size = 512;

compile_shader_res compile_shader(const GLenum type, const char *source) {
  int success;
  char info_log[shader_info_log_size];

  id_t shader;
  shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, shader_info_log_size, nullptr, info_log);
    auto error = std::format("error shader {}, compilation failed\n{}",
                             type_to_view(type), info_log);
    return std::unexpected(error);
  }
  return shader;
}

std::expected<id_t, std::string> link_shaders(std::vector<id_t> shaders) {
  int success;
  char info_log[shader_info_log_size];

  id_t program;
  program = glCreateProgram();
  for (auto shader : shaders) {
    glAttachShader(program, shader);
  }
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, shader_info_log_size, nullptr, info_log);
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

read_file_res read_file(const std::filesystem::path &path) {
  try {
    std::ifstream file(path, std::ios::binary);
    if (!file)
      return std::unexpected(
          std::format("error while trying to read file {}", path.string()));
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    auto size = std::filesystem::file_size(path);
    std::string content(size, '\0');
    file.read(content.data(), size);
    return content;

  } catch (std::ifstream::failure e) {
    return std::unexpected(std::format("error reading shader file \"{}\"\n{}",
                                       path.string(), e.what()));
  }
}
void set_int(id_t id, const std::string &name, int value) {
  GLint location = glGetUniformLocation(id, name.c_str());
  glUniform1i(location, value);
}

void set_float(id_t id, const std::string &name, float value) {
  GLint location = glGetUniformLocation(id, name.c_str());
  glUniform1f(location, value);
}

void set_vec3(id_t id, const std::string &name, const glm::vec3 &value) {
  GLint location = glGetUniformLocation(id, name.c_str());
  glUniform3fv(location, 1, glm::value_ptr(value));
}

void set_vec4(id_t id, const std::string &name, const glm::vec4 &value) {
  GLint location = glGetUniformLocation(id, name.c_str());
  glUniform4fv(location, 1, glm::value_ptr(value));
}

void set_mat4(id_t id, const std::string &name, const glm::mat4 &value) {
  GLint location = glGetUniformLocation(id, name.c_str());
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}
