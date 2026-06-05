#version 450

layout(location = 0) in vec2 frag_tex_coord;

layout(set = 2, binding = 0) uniform sampler2D texture1;
layout(set = 2, binding = 1) uniform sampler2D texture2;

// objectColor and lightColor in vec4 to satisfy std140 alignment; only .rgb is used.
layout(set = 3, binding = 0) uniform Lighting {
    vec4 object_color;
    vec4 light_color;
};

layout(location = 0) out vec4 frag_color;

void main() {
    vec4 tex = mix(texture(texture1, frag_tex_coord), texture(texture2, frag_tex_coord), 0.2);
    frag_color = vec4(light_color.rgb * object_color.rgb, 1.0) * tex;
}
