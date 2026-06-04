#version 450

layout(location = 0) in vec3 position;

// set=1 is the SDL3_GPU convention for vertex uniform buffers
layout(set = 1, binding = 0) uniform Rotation {
    float angle;
};

void main() {
    float c = cos(angle);
    float s = sin(angle);
    gl_Position = vec4(
        position.x * c - position.y * s,
        position.x * s + position.y * c,
        position.z,
        1.0
    );
}
