#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <expected>
#include <string>
#include <vector>

using id_t = unsigned int;

class Shader {
public:
  // the program ID
  id_t ID;

  // constructor reads and builds the shader
  using build_res = std::expected<Shader, std::string>;
  static build_res build(std::string_view vertexPath,
                         std::string_view fragmentPath);
  // use/activate the shader
  void use();
  // utility uniform functions
  void set_bool(const std::string &name, bool value) const;
  void set_int(const std::string &name, int value) const;
  void set_float(const std::string &name, float value) const;
  void set_vec3(const std::string &name, const glm::vec3 &value) const;
};

using compile_shader_res = std::expected<id_t, std::string>;
compile_shader_res compile_shader(const GLenum type, const char *source);

using link_shaders_res = std::expected<id_t, std::string>;
link_shaders_res link_shaders(const std::vector<unsigned int> shaders);

void set_vec3(const unsigned int id, const std::string &name,
              const glm::vec3 &value);
void set_vec4(const unsigned int id, const std::string &name,
              const glm::vec4 &value);
void set_mat4(const unsigned int id, const std::string &name,
              const glm::mat4 &value);
