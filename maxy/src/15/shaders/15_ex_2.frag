#version 330 core

out vec4 frag_color;

in vec2 tex_coord;
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos_view;

struct specular_map_t {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct light_t {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform specular_map_t specular_map;
uniform light_t light;

void main() {
  vec3 ambient = light.ambient * texture(specular_map.diffuse, tex_coord).rgb;

  vec3 norm = normalize(normal);
  vec3 light_dir = normalize(light_pos_view - frag_pos);
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light.diffuse * diff * texture(specular_map.diffuse, tex_coord).rgb;

  vec3 view_dir = normalize(-frag_pos);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), specular_map.shininess);
  vec3 specular = light.specular * spec * (vec3(1.0) - texture(specular_map.specular, tex_coord).rgb);

  vec3 result = ambient + diffuse + specular;
  frag_color = vec4(result, 1.0);
}
