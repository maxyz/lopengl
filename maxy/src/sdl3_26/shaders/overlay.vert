#version 460 core

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_tex_coord;

layout(location = 0) out vec2 frag_tex_coord;

layout(set = 1, binding = 0) uniform Transform {
    mat4 transform;
};

void main() {
    frag_tex_coord = a_tex_coord;
    gl_Position = transform * vec4(a_position, 0.0, 1.0);
}
