#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 2) in vec2 a_tex_coords;

out vs_out_t {
    vec2 tex_coords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vs_out.tex_coords = a_tex_coords;
    gl_Position = projection * view * model * vec4(a_pos, 1.0); 
}
