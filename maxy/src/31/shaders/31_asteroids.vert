#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 2) in vec2 a_tex_coords;
layout (location = 3) in mat4 a_instance_matrix;

out vec2 tex_coords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    tex_coords = a_tex_coords;
    gl_Position = projection * view * a_instance_matrix * vec4(a_pos, 1.0f); 
}
