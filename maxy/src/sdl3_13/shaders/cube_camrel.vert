#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

layout(set = 1, binding = 0) uniform Model {
    mat4 model;
};
// View is rotation-only (translation stripped on CPU). Positions are already
// camera-relative, so the view matrix only needs to orient axes for projection.
layout(set = 1, binding = 1) uniform View {
    mat4 view;
};
layout(set = 1, binding = 2) uniform Projection {
    mat4 projection;
};

layout(location = 0) out vec2 frag_tex_coord;
layout(location = 1) out vec3 frag_pos; // camera-relative world space
layout(location = 2) out vec3 frag_normal; // camera-relative world space

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    frag_pos = vec3(model * vec4(position, 1.0));
    frag_tex_coord = tex_coord;
    // View is rotation-only, so the normal matrix needs only the model linear part.
    // Equivalent to world-space normal matrix but with camera-relative translation.
    frag_normal = mat3(transpose(inverse(model))) * normal;
}
