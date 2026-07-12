#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_color;

out vs_out_t {
    vec4 color;
} vs_out;

void main() {
    vs_out.color = a_color;
    gl_Position = vec4(a_pos, 1.0); 
}
