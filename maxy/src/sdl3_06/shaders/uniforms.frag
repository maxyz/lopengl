#version 450

// set=3 is the SDL3_GPU convention for fragment uniform buffers
layout(set = 3, binding = 0) uniform ColorBlock {
    vec4 our_color;
};

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = our_color;
}
