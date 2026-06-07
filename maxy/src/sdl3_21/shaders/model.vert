#version 460 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coords;

layout(set = 1, binding = 0) uniform ModelBlock {
    mat4 model;
};
layout(set = 1, binding = 1) uniform ViewBlock {
    mat4 view;
};
layout(set = 1, binding = 2) uniform ProjectionBlock {
    mat4 projection;
};

layout(location = 0) out vec2 tex_coords;

void main() {
    tex_coords = a_tex_coords;
    gl_Position = projection * view * model * vec4(a_pos, 1.0);
}
