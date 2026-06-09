#version 460 core

layout(location = 0) in vec2 tex_coords;

layout(set = 2, binding = 0) uniform sampler2D texture1;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = texture(texture1, tex_coords);
}
