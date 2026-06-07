#version 450

// Solid-colour indicator for positional and spot light positions.
// The CPU pushes the light's ambient+diffuse sum as a single vec4.
layout(set = 3, binding = 0) uniform ColorBlock {
    vec4 color;
} indicator;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = indicator.color;
}
