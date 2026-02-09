#include "shader.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

using read_file_res = std::expected<std::string, std::string>;
read_file_res read_file(const std::filesystem::path &path);

Shader::build_res Shader::build(std::string_view vertexPath,
                                std::string_view fragmentPath) {
  std::filesystem::path vPath(vertexPath);
  auto vCode = read_file(vPath);
  if (!vCode.has_value()) {
    return std::unexpected(vCode.error());
  }
  std::filesystem::path fPath(fragmentPath);
  auto fCode = read_file(fPath);
  if (!fCode.has_value()) {
    return std::unexpected(fCode.error());
  }

  const char *vShaderCode = vCode.value().c_str();
  const char *fShaderCode = fCode.value().c_str();

  // 2. compile shaders
  auto v_res = compile_shader(GL_VERTEX_SHADER, vShaderCode);
  if (!v_res.has_value()) {
    return std::unexpected(v_res.error());
  }
  auto f_res = compile_shader(GL_FRAGMENT_SHADER, fShaderCode);
  if (!f_res.has_value()) {
    return std::unexpected(f_res.error());
  }

  auto p_res = link_shaders({v_res.value(), f_res.value()});
  if (!p_res.has_value()) {
    return std::unexpected(p_res.error());
  }
  // delete shaders; they're linked into our program and no longer necessary
  glDeleteShader(v_res.value());
  glDeleteShader(p_res.value());

  return Shader{p_res.value()};
};

void Shader::use() { glUseProgram(ID); }

void Shader::setBool(const std::string &name, bool value) const {
  glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string &name, int value) const {
  glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const {
  glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

const std::string_view type_to_view(const GLenum type);

compile_shader_res compile_shader(const GLenum type, const char *source) {
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
