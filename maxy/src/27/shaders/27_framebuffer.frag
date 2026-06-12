#version 330 core
out vec4 frag_color;

in vec2 tex_coords;

uniform sampler2D screen_texture;
uniform bool effects[8];

const int EFFECT_INVERT = 0;
const int EFFECT_GREYSCALE = 1;
const int EFFECT_EXAMPLE = 2;
const int EFFECT_NARCO = 3;
const int EFFECT_BLUR = 4;
const int EFFECT_EDGE = 5;

vec4 invert(vec4 color) {
    return vec4(vec3(1.0 - color), 1.0);
}

vec4 greyscale(vec4 color) {
    float average = (0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b) / 3.0;
    return vec4(average, average, average, 1.0);
}

const float offset = 1. / 300.;
const vec2 offsets[9] = vec2[](
        vec2(-offset, offset),
        vec2(0, offset),
        vec2(offset, offset),
        vec2(-offset, 0),
        vec2(0, 0),
        vec2(offset, 0),
        vec2(-offset, -offset),
        vec2(0, -offset),
        vec2(offset, -offset)
    );

const float ex_kernel[9] = float[](
        2, 2, 2,
        2, -15, 2,
        2, 2, 2
    );
const float narco_kernel[9] = float[](
        -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1
    );
const float blur_kernel[9] = float[](
        1. / 16, 2. / 16, 1. / 16,
        2. / 16, 4. / 16, 2. / 16,
        1. / 16, 2. / 16, 1. / 16
    );
const float edge_kernel[9] = float[](
        -1, -1, -1,
        -1, 9, -1,
        -1, -1, -1
    );

vec4 apply_kernel(in float kernel[9]) {
    vec3
    sample [9];
    for (int i = 0; i < 9; ++i) {
        sample [i] = vec3(texture(screen_texture, tex_coords.st+offsets[i]));
}
vec3 color = vec3(0.);
for ( int i = 0; i < 9 ; ++ i ) {
color += sample [ i ] * kernel[i];
}

return vec4(color, 1.0);
}

void main() {
    vec4 color;
    if (effects[EFFECT_EXAMPLE]) {
        color = apply_kernel(ex_kernel);
    } else if (effects[EFFECT_NARCO]) {
        color = apply_kernel(narco_kernel);
    } else if (effects[EFFECT_BLUR]) {
        color = apply_kernel(blur_kernel);
    } else if (effects[EFFECT_EDGE]) {
        color = apply_kernel(edge_kernel);
    } else {
        color = vec4(texture(screen_texture, tex_coords).rgb, 1.0);
    }
    if (effects[EFFECT_INVERT]) {
        color = invert(color);
    }
    if (effects[EFFECT_GREYSCALE]) {
        color = greyscale(color);
    }

    frag_color = color;
}
