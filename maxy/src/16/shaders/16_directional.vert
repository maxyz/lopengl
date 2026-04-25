#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;

out vec2 tex_coord;
out vec3 frag_pos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  mat4 vm = view * model;
  vec4 vm_pos = vm * vec4(a_pos, 1.0);

  tex_coord = a_tex_coord;
  frag_pos = vec3(vm_pos);
  normal = mat3(transpose(inverse(vm))) * a_normal;

  gl_Position = projection * vm_pos;
}
