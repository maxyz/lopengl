#version 450

layout(location = 0) in vec3 tex_coord;

layout(location = 0) out vec4 frag_color;

layout(set = 2, binding = 0) uniform samplerCube skybox;

void main() {
    frag_color = texture(skybox, tex_coord);
}
