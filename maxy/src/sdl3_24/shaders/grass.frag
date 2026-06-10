#version 460 core

layout(location = 0) in vec2 frag_tex_coord;

layout(set = 2, binding = 0) uniform sampler2D diffuse_tex;

layout(location = 0) out vec4 frag_color;

void main() {
    vec4 color = texture(diffuse_tex, frag_tex_coord);
    if (color.a < 0.1) discard;
    frag_color = color;
}
