#ifndef LOPENGL_SHADER_H
#define LOPENGL_SHADER_H

#include <glad/gl.h>

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
  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;
};

using compile_shader_res = std::expected<id_t, std::string>;
compile_shader_res compile_shader(const GLenum type, const char *source);

using link_shaders_res = std::expected<id_t, std::string>;
link_shaders_res link_shaders(const std::vector<unsigned int> shaders);

#endif
