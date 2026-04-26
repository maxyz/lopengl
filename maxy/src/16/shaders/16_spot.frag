#version 330 core

out vec4 frag_color;

in vec2 tex_coord;
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos_view;
in vec3 light_dir_view;

struct material_t {
  sampler2D diffuse;
  sampler2D specular;

  float shininess;
};

struct spot_light_t {
  vec3 position;
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float cutoff;
  float outer_cutoff;

  float constant;
  float linear;
  float quadratic;
};

uniform material_t material;
uniform spot_light_t light;

void main() {
  vec3 ambient = light.ambient * texture(material.diffuse, tex_coord).rgb;

  vec3 light_dir = normalize(light_pos_view - frag_pos);
  float theta = dot(light_dir, normalize(-light_dir_view));
  float epsilon = light.cutoff - light.outer_cutoff;
  float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0., 1.);

  vec3 norm = normalize(normal);
  float diff = max(dot(norm, light_dir), 0.);
  vec3 diffuse = light.diffuse * diff * texture(material.diffuse, tex_coord).rgb;

  vec3 view_dir = normalize(-frag_pos);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.), material.shininess);
  vec3 specular = light.specular * spec * texture(material.specular, tex_coord).rgb;

  float distance = length(light_pos_view - frag_pos);
  float attenuation = 1. / (light.constant + light.linear * distance + light.quadratic * distance * distance);

  vec3 result = attenuation * (ambient + (intensity * (diffuse + specular)));
  frag_color = vec4(result, 1.);
}
