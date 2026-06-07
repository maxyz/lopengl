#version 450

// Reads SpotLightBlock at binding 0 (same layout as spot.frag binding 1).
// Only ambient and diffuse are used for the indicator colour.
layout(set = 3, binding = 0) uniform SpotLightBlock {
    vec4 position;
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float cutoff;
    float outer_cutoff;
    float constant;
    float linear;
    float quadratic;
} light;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = vec4(light.ambient.rgb + light.diffuse.rgb, 1.0);
}
