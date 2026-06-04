#version 450

layout(location = 0) in vec3 vertex_color;
layout(location = 1) in vec2 frag_tex_coord;

layout(set = 2, binding = 0) uniform sampler2D texture1; // container - clamp_to_edge
layout(set = 2, binding = 1) uniform sampler2D texture2; // face - repeat

layout(location = 0) out vec4 frag_color;

void main() {
    // texture1 sampled normally -- clamp_to_edge wrapping stretches the edge pixels
    // texture2 scaled to 0..2 with x mirrored -- repeat wrapping tiles 4 faces
    vec2 tiled = vec2(1.0 - frag_tex_coord.x, frag_tex_coord.y) * 2.0;
    frag_color = mix(texture(texture1, frag_tex_coord), texture(texture2, tiled), 0.2);
}
