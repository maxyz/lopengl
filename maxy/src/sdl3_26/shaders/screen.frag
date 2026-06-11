#version 460 core

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 0) out vec4 out_color;

layout(set = 2, binding = 0) uniform sampler2D screen_texture;

// Bit flags: bits 0-2 = kernel index (0=none,1=blur,2=edge,3=sharpen,4=emboss),
//            bit 3 = invert, bit 4 = greyscale.
layout(set = 3, binding = 0) uniform EffectFlags {
    uint flags;
};

vec2 texel_size() {
    return 1.0 / vec2(textureSize(screen_texture, 0));
}

const vec2 NEIGHBOR_OFFSETS[9] = vec2[](
        vec2(-1, 1), vec2(0, 1), vec2(1, 1),
        vec2(-1, 0), vec2(0, 0), vec2(1, 0),
        vec2(-1, -1), vec2(0, -1), vec2(1, -1)
    );

vec4 apply_kernel(float k[9]) {
    vec2 ts = texel_size();
    vec4 result = vec4(0.0);
    for (int i = 0; i < 9; ++i)
        result += texture(screen_texture, frag_tex_coord + NEIGHBOR_OFFSETS[i] * ts) * k[i];
    return result;
}

void main() {
    uint kernel_idx = flags & 0x7u;
    bool do_invert = (flags & 0x8u) != 0u;
    bool do_grey = (flags & 0x10u) != 0u;

    vec4 color;
    if (kernel_idx == 1u) {
        float k[9] = float[](
                1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
                2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
                1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0
            );
        color = apply_kernel(k);
    } else if (kernel_idx == 2u) {
        float k[9] = float[](
                1.0, 1.0, 1.0,
                1.0, -8.0, 1.0,
                1.0, 1.0, 1.0
            );
        color = apply_kernel(k);
    } else if (kernel_idx == 3u) {
        float k[9] = float[](
                0.0, -1.0, 0.0,
                -1.0, 5.0, -1.0,
                0.0, -1.0, 0.0
            );
        color = apply_kernel(k);
    } else if (kernel_idx == 4u) {
        float k[9] = float[](
                -2.0, -1.0, 0.0,
                -1.0, 1.0, 1.0,
                0.0, 1.0, 2.0
            );
        color = apply_kernel(k);
    } else {
        color = texture(screen_texture, frag_tex_coord);
    }

    if (do_invert)
        color.rgb = 1.0 - color.rgb;

    if (do_grey) {
        float grey = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        color.rgb = vec3(grey);
    }

    out_color = color;
}
