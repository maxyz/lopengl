#version 450

layout(location = 0) in vec2 frag_tex_coord;
// Per-vertex Phong intensity interpolated across the triangle.
layout(location = 1) in vec3 light_gouraud;

layout(set = 2, binding = 0) uniform sampler2D tex1;
layout(set = 2, binding = 1) uniform sampler2D tex2;

layout(set = 3, binding = 0) uniform FragmentLighting {
    vec4 object_color;
};

layout(location = 0) out vec4 frag_color;

void main() {
    vec4 tex = mix(texture(tex1, frag_tex_coord), texture(tex2, frag_tex_coord), 0.2);
    frag_color = vec4(light_gouraud * object_color.rgb, 1.0) * tex;
}
