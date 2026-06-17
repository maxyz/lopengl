#version 460 core

// Flat mirror with an emissive border. The reflective interior reflects the
// camera-to-fragment direction off the surface normal and samples the mirror's
// cubemap (rendered from the mirror's centre); paired with lit.vert, frag_pos
// is camera-relative so normalize(frag_pos) is the world-space incident ray.
//
// The bright border makes the mirror visible against the dark, otherwise-empty
// scene; reflected recursively, the borders form the receding tunnel of frames
// that reads as an infinity mirror.
layout(location = 0) in vec2 frag_tex_coord; // quad UV in [0,1]
layout(location = 1) in vec3 frag_pos; // camera-relative
layout(location = 2) in vec3 frag_normal; // world orientation

layout(set = 2, binding = 0) uniform samplerCube reflection_cube;

layout(location = 0) out vec4 frag_color;

const float BORDER = 0.05;
const vec3 FRAME_COLOR = vec3(0.25, 0.85, 1.0);

void main() {
    vec2 uv = frag_tex_coord;
    if (uv.x < BORDER || uv.x > 1.0 - BORDER || uv.y < BORDER || uv.y > 1.0 - BORDER) {
        frag_color = vec4(FRAME_COLOR, 1.0);
        return;
    }
    vec3 incident = normalize(frag_pos);
    vec3 normal = normalize(frag_normal);
    vec3 reflect_dir = reflect(incident, normal);
    frag_color = vec4(texture(reflection_cube, reflect_dir).rgb, 1.0);
}
