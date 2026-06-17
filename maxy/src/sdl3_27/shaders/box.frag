#version 460 core

// Camera-relative lit box (paired with lit.vert, which bakes -camera_position
// into the model matrix so the camera sits at the origin). One directional
// light is enough for this minimal scene.
layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos; // camera-relative
layout(location = 2) in vec3 frag_normal; // world orientation

layout(set = 2, binding = 0) uniform sampler2D diffuse_tex;

layout(set = 3, binding = 0) uniform LightBlock {
    vec4 light_direction; // world-space direction the light travels (w unused)
};

layout(location = 0) out vec4 frag_color;

void main() {
    vec3 base = texture(diffuse_tex, frag_tex_coord).rgb;
    vec3 normal = normalize(frag_normal);
    vec3 light_dir = normalize(-light_direction.xyz);
    float diffuse = max(dot(normal, light_dir), 0.0);
    vec3 view_dir = normalize(-frag_pos); // camera is at the origin
    vec3 halfway = normalize(light_dir + view_dir);
    float specular = pow(max(dot(normal, halfway), 0.0), 32.0);
    vec3 color = base * (0.25 + 0.75 * diffuse) + vec3(0.25) * specular;
    frag_color = vec4(color, 1.0);
}
