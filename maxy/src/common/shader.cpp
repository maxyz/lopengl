#include "common/shader.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

using read_file_res = std::expected<std::string, std::string>;
read_file_res read_file(const std::filesystem::path &path);

static std::string_view type_to_view(GLenum type) {
    switch (type) {
    case GL_VERTEX_SHADER:
        return "vertex";
    case GL_FRAGMENT_SHADER:
        return "fragment";
    default:
        return "unknown";
    }
}

static compile_shader_res compile_file(std::string_view path, GLenum type) {
    auto code = read_file(fs::path(path));
    if (!code) {
        return std::unexpected(std::format("error reading {}: {}", path, code.error()));
    }
    auto object = compile_shader(type, code->c_str());
    if (!object) {
        return std::unexpected(std::format("error compiling {}: {}", path, object.error()));
    }
    return object;
}

Shader::build_res Shader::build(std::string_view vertexPath, std::string_view fragmentPath) {
    auto vertex = compile_file(vertexPath, GL_VERTEX_SHADER);
    if (!vertex) {
        return std::unexpected(vertex.error());
    }
    auto fragment = compile_file(fragmentPath, GL_FRAGMENT_SHADER);
    if (!fragment) {
        glDeleteShader(*vertex);
        return std::unexpected(fragment.error());
    }
    const std::array ids{*vertex, *fragment};
    auto             program = link_shaders(ids);
    glDeleteShader(*vertex);
    glDeleteShader(*fragment);
    if (!program) {
        return std::unexpected(program.error());
    }
    return Shader(*program);
}

void Shader::use() {
    glUseProgram(m_program_id);
}

void Shader::set_bool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(m_program_id, name.c_str()), static_cast<int>(value));
}
void Shader::set_int(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(m_program_id, name.c_str()), value);
}
void Shader::set_float(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(m_program_id, name.c_str()), value);
}
void Shader::set_vec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(m_program_id, name.c_str()), 1, glm::value_ptr(value));
}
void Shader::set_vec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(m_program_id, name.c_str()), 1, glm::value_ptr(value));
}
void Shader::set_mat4(const std::string &name, const glm::mat4 &value) const {
    glUniformMatrix4fv(
        glGetUniformLocation(m_program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)
    );
}

constexpr int shader_info_log_size = 512;

compile_shader_res compile_shader(GLenum type, const char *source) {
    int  success;
    char info_log[shader_info_log_size];

    id_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, shader_info_log_size, nullptr, info_log);
        return std::unexpected(
            std::format("error shader {}, compilation failed\n{}", type_to_view(type), info_log)
        );
    }
    return shader;
}

link_shaders_res link_shaders(std::span<const id_t> shaders) {
    int  success;
    char info_log[shader_info_log_size];

    id_t program = glCreateProgram();
    for (auto shader : shaders) {
        glAttachShader(program, shader);
    }
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, shader_info_log_size, nullptr, info_log);
        return std::unexpected(std::format("error shader link failed\n{}", info_log));
    }
    return program;
}

read_file_res read_file(const std::filesystem::path &path) {
    try {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            return std::unexpected(
                std::format("error while trying to read file {}", path.string())
            );
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        auto        size = std::filesystem::file_size(path);
        std::string content(size, '\0');
        file.read(content.data(), size);
        return content;

    } catch (const std::ifstream::failure &e) {
        return std::unexpected(
            std::format("error reading shader file \"{}\"\n{}", path.string(), e.what())
        );
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
