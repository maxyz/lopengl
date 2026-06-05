#version 450

layout(location = 0) in vec2 frag_tex_coord;

layout(set = 2, binding = 0) uniform sampler2D texture1;
layout(set = 2, binding = 1) uniform sampler2D texture2;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = mix(texture(texture1, frag_tex_coord),
            texture(texture2, frag_tex_coord),
            0.2);
}
