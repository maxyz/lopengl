#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_coord;

// Separate bindings allow updating model (slot 0) per draw while keeping
// view (slot 1) and projection (slot 2) constant across multiple draws.
layout(set = 1, binding = 0) uniform Model {
    mat4 model;
};
layout(set = 1, binding = 1) uniform View {
    mat4 view;
};
layout(set = 1, binding = 2) uniform Projection {
    mat4 projection;
};

layout(location = 0) out vec2 frag_tex_coord;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    frag_tex_coord = tex_coord;
}
