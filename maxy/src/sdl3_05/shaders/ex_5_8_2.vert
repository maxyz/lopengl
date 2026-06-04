#version 450

layout(location = 0) in vec3 position;

layout(set = 1, binding = 0) uniform Translate {
    vec2 offset;
};

void main() {
    gl_Position = vec4(position.xy + offset, position.z, 1.0);
}
