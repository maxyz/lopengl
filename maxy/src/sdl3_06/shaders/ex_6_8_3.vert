#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color; // declared but not used -- position is the color

layout(set = 1, binding = 0) uniform Offset {
    vec2 offset;
};

layout(location = 0) out vec3 vertex_color;

void main() {
    gl_Position = vec4(position.xy + offset, position.z, 1.0);
    // Output position (after offset) as colour.  Negative values clamp to 0,
    // which is why the bottom-left vertex is black: both x and y are negative
    // there, giving rgb(0, 0, 0).
    vertex_color = vec3(position.xy + offset, position.z);
}
