#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;

out vec2 tex_coord;
out vec3 frag_pos;
out vec3 normal;
out vec3 light_pos_view;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct light_t {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};
uniform light_t light;

void main() {
  gl_Position = projection * view * model * vec4(a_pos, 1.0);
  frag_pos = vec3(view * model * vec4(a_pos, 1.0));
  tex_coord = a_tex_coord;
  normal = mat3(transpose(inverse(view * model))) * a_normal;
  light_pos_view = vec3(view * vec4(light.position, 1.0));
}
