#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color; // present in buffer, not used here
layout(location = 2) in vec2 tex_coord;

// set=1: vertex uniform buffer
layout(set = 1, binding = 0) uniform Transform {
    mat4 transform;
};

layout(location = 0) out vec2 frag_tex_coord;

void main() {
    gl_Position = transform * vec4(position, 1.0);
    frag_tex_coord = tex_coord;
}
