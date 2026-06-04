#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 tex_coord;

layout(location = 0) out vec3 vertex_color;
layout(location = 1) out vec2 frag_tex_coord;

void main() {
    gl_Position = vec4(position, 1.0);
    vertex_color = color;
    frag_tex_coord = tex_coord;
}
