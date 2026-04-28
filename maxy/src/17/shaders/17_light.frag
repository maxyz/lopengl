#version 330 core

out vec4 frag_color;

struct light_t {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform light_t light;

void main() {
  vec3 color = light.ambient + light.diffuse + light.specular;
  frag_color = vec4(color, 1.0);
}
