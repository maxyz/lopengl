#version 450

layout(location = 0) in vec3 a_pos;

layout(location = 0) out vec3 tex_coord;

layout(set = 1, binding = 0) uniform ViewUniform {
    mat4 view;
};
layout(set = 1, binding = 1) uniform ProjUniform {
    mat4 proj;
};

void main() {
    tex_coord = a_pos;
    vec4 pos = proj * view * vec4(a_pos, 1.0);
    // Force depth to 1.0 after perspective divide: z/w = w/w = 1.0
    gl_Position = pos.xyww;
}
