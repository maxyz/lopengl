#version 450

layout(location = 0) in vec3 vertex_color;
layout(location = 1) in vec2 frag_tex_coord;

// set=2 is the SDL3_GPU convention for fragment samplers
layout(set = 2, binding = 0) uniform sampler2D our_texture;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = texture(our_texture, frag_tex_coord) * vec4(vertex_color, 1.0);
}
