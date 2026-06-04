#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

// set=1 is the SDL3_GPU convention for vertex uniform buffers
layout(set = 1, binding = 0) uniform Offset {
    vec2 offset;
};

layout(location = 0) out vec3 vertex_color;

void main() {
    gl_Position = vec4(position.xy + offset, position.z, 1.0);
    vertex_color = color;
}
