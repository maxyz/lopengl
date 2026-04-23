#version 330 core

out vec4 frag_color;

in vec2 tex_coord;
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos_view;

struct emission_map_t {
  sampler2D diffuse;
  sampler2D specular;
  sampler2D emission;
  float shininess;
};

struct light_t {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform emission_map_t emission_map;
uniform light_t light;

void main() {
  vec3 ambient = light.ambient * texture(emission_map.diffuse, tex_coord).rgb;

  vec3 norm = normalize(normal);
  vec3 light_dir = normalize(light_pos_view - frag_pos);
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light.diffuse * diff * texture(emission_map.diffuse, tex_coord).rgb;

  vec3 view_dir = normalize(-frag_pos);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), emission_map.shininess);
  vec3 specular = light.specular * spec * texture(emission_map.specular, tex_coord).rgb;

  vec3 emission = texture(emission_map.emission, tex_coord).rgb;

  vec3 result = ambient + diffuse + specular + emission;
  frag_color = vec4(result, 1.0);
}
