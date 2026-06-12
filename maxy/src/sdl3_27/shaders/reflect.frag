#version 460 core

// frag_pos and frag_normal come from lit.vert.
// frag_pos is world-space relative to the current camera position
// (model matrix bakes in -cam_pos), so normalize(frag_pos) is the
// world-space incident ray from the camera to the fragment --
// identical convention to environment.frag.
layout(location = 1) in vec3 frag_pos;
layout(location = 2) in vec3 frag_normal;

layout(set = 2, binding = 0) uniform samplerCube dynamic_cubemap;

layout(location = 0) out vec4 frag_color;

void main() {
    vec3 norm = normalize(frag_normal);
    vec3 incident = normalize(frag_pos);
    vec3 reflect_dir = reflect(incident, norm);
    frag_color = vec4(texture(dynamic_cubemap, reflect_dir).rgb, 1.0);
}
