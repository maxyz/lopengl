#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coords;

out vec2 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float border_thickness;

void main() {
  vec4 pos_clip    = projection * view * model * vec4(a_pos, 1.0);
  vec3 normal_view = normalize(mat3(transpose(inverse(view * model))) * a_normal);
  vec2 normal_ndc  = normalize((projection * vec4(normal_view, 0.0)).xy);
  gl_Position = pos_clip + vec4(normal_ndc * border_thickness * pos_clip.w, 0.0, 0.0);
  tex_coords = a_tex_coords;
}
