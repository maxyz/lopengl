#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;

out vec2 tex_coord;
out vec3 light_gouraud;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_pos;

uniform vec3 light_color;
// ambient_strength, diffuse_strength, specular_strength, shininess
uniform vec4 light_strengths = vec4(
    0.1,
    1.0,
    0.5,
    32
  );

void main() {
  gl_Position = projection * view * model * vec4(a_pos, 1.0);

  vec3 frag_pos = vec3(view * model * vec4(a_pos, 1.0));
  vec3 normal = mat3(transpose(inverse(view * model))) * a_normal;
  vec3 light_pos_view = vec3(view * vec4(light_pos, 1.0));

  vec3 ambient = light_strengths.x * light_color;

  vec3 norm = normalize(normal);
  vec3 light_dir = normalize(light_pos_view - frag_pos);
  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light_strengths.y * diff * light_color;

  vec3 view_dir = normalize(-frag_pos);
  vec3 reflect_dir = reflect(-light_dir, norm);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light_strengths.w);
  vec3 specular = light_strengths.z * spec * light_color;

  tex_coord = a_tex_coord;
  light_gouraud = ambient + diffuse + specular;
}
