#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

layout(set = 1, binding = 0) uniform Model {
    mat4 model;
};
layout(set = 1, binding = 1) uniform View {
    mat4 view;
};
layout(set = 1, binding = 2) uniform Projection {
    mat4 projection;
};
// Light position in world space; transformed to view space here so the fragment
// shader can work entirely in view space without a separate view_pos uniform.
layout(set = 1, binding = 3) uniform LightPos {
    vec4 light_pos;
};

layout(location = 0) out vec2 frag_tex_coord;
layout(location = 1) out vec3 frag_pos; // view space
layout(location = 2) out vec3 frag_normal; // view space
layout(location = 3) out vec3 light_pos_view; // view space

void main() {
    mat4 view_model = view * model;
    gl_Position = projection * view_model * vec4(position, 1.0);
    frag_pos = vec3(view_model * vec4(position, 1.0));
    frag_tex_coord = tex_coord;
    // Normal matrix includes view so normals stay correct in view space.
    frag_normal = mat3(transpose(inverse(view_model))) * normal;
    light_pos_view = vec3(view * vec4(light_pos.xyz, 1.0));
}
