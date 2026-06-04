#version 450

layout(location = 0) in vec3 vertex_color;
layout(location = 1) in vec2 frag_tex_coord;

layout(set = 2, binding = 0) uniform sampler2D texture1;
layout(set = 2, binding = 1) uniform sampler2D texture2;

layout(location = 0) out vec4 frag_color;

void main() {
    // Mirror the face texture horizontally while keeping the container normal.
    vec2 mirrored = vec2(1.0 - frag_tex_coord.x, frag_tex_coord.y);
    frag_color = mix(texture(texture1, frag_tex_coord),
                     texture(texture2, mirrored),
                     0.2);
}
