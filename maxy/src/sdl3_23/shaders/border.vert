#version 460 core

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
layout(set = 1, binding = 3) uniform BorderParams {
    float thickness;
};

void main() {
    vec4 pos_clip = projection * view * model * vec4(position, 1.0);
    // Normal in view space; view is rotation-only in camera-relative convention.
    vec3 normal_view = normalize(mat3(transpose(inverse(view * model))) * normal);
    // Project normal to NDC, then expand clip-space position along it.
    // Multiplying by pos_clip.w keeps border width constant in screen space at any depth.
    vec2 normal_ndc = normalize((projection * vec4(normal_view, 0.0)).xy);
    gl_Position = pos_clip + vec4(normal_ndc * thickness * pos_clip.w, 0.0, 0.0);
}
