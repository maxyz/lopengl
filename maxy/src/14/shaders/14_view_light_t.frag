#version 330 core

out vec4 frag_color;

in vec2 tex_coord;
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos_view;

uniform sampler2D texture1;
uniform sampler2D texture2;

struct material_t {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};
struct light_t {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform material_t material;
uniform light_t light;

void main() {
  vec3 ambient = light.ambient * material.ambient;

  vec3 norm = normalize(normal);
  vec3 light_dir = normalize(light_pos_view - frag_pos);
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light.diffuse * diff * material.diffuse;

  vec3 view_dir = normalize(-frag_pos);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light.specular * spec * material.specular;

  vec3 intensity = ambient + diffuse + specular;

  frag_color = vec4(intensity, 1.0) * mix(texture(texture1, tex_coord), texture(texture2, tex_coord), 0.2);
}
