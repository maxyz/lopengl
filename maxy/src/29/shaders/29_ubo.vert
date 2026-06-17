#version 330 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_norm;
layout(location = 2) in vec3 a_tex_coord;

layout(std140) uniform matrices {
    mat4 projection;
    mat4 view;
};
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(a_pos, 1.0);
}
