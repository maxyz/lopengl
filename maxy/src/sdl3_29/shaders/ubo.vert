#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_coord; // present in vertex layout; unused

layout(set = 1, binding = 0) uniform Model {
    mat4 model;
};
layout(set = 1, binding = 1) uniform View {
    mat4 view;
};
layout(set = 1, binding = 2) uniform Projection {
    mat4 projection;
};

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
}
