#version 330 core

out vec4 frag_color;

in vec2 tex_coord;
in vec3 frag_pos;
in vec3 normal;

struct material_t {
  sampler2D diffuse;
  sampler2D specular;

  float shininess;
};

struct directional_light_t {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform material_t material;
uniform directional_light_t light;

void main() {
  vec3 ambient = light.ambient * texture(material.diffuse, tex_coord).rgb;

  vec3 norm = normalize(normal);
  vec3 light_dir = normalize(-light.direction);
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light.diffuse * diff * texture(material.diffuse, tex_coord).rgb;

  vec3 view_dir = normalize(-frag_pos);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light.specular * spec * texture(material.specular, tex_coord).rgb;

  vec3 result = ambient + diffuse + specular;
  frag_color = vec4(result, 1.0);
}
