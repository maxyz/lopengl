#version 330 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex_coords;

out vec2 tex_coords;

uniform mat4 transform = mat4(1.);

void main() {
    tex_coords = a_tex_coords;
    gl_Position = transform * vec4(a_pos.x, a_pos.y, 0.0, 1.0);
}
