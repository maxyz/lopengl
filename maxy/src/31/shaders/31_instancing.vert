#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec3 a_offset;

out vec4 f_color;

void main() {
    f_color = a_color;
    gl_Position = vec4(a_pos + a_offset, 1.0);
}
