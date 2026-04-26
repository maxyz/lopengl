#version 330 core

out vec4 frag_color;

struct positional_light_t {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};
uniform positional_light_t light;

void main() {
  vec3 color = light.ambient + light.diffuse;
  frag_color = vec4(color, 1.0);
}
