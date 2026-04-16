#version 330 core

out vec4 frag_color;

in vec2 tex_coord;
in vec3 frag_pos;
in vec3 normal;
in vec3 light_pos_view;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 object_color;
uniform vec3 light_color;
// ambient_strength, diffuse_strength, specular_strength, shininess
uniform vec4 light_strengths = vec4(
    0.1,
    1.0,
    0.5,
    32
  );

void main() {
  vec3 ambient = light_strengths.x * light_color;

  vec3 norm = normalize(normal);
  vec3 light_dir = normalize(light_pos_view - frag_pos);
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light_strengths.y * diff * light_color;

  vec3 view_dir = normalize(-frag_pos);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light_strengths.w);
  vec3 specular = light_strengths.z * spec * light_color;

  vec3 intensity = ambient + diffuse + specular;

  frag_color = vec4(intensity * object_color, 1.0) * mix(texture(texture1, tex_coord), texture(texture2, tex_coord), 0.2);
}
