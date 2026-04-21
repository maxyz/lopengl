#version 330 core

layout(location = 0) in vec3 a_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

struct material_t {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

void main() {
  gl_Position = projection * view * model * vec4(a_pos, 1.0);
}
