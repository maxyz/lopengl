#version 450

// Reads PositionalLightBlock at binding 0 (same struct as positional.frag binding 1).
// Only ambient and diffuse are used; the rest is ignored.
layout(set = 3, binding = 0) uniform PositionalLightBlock {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
} light;

layout(location = 0) out vec4 frag_color;

void main() {
    frag_color = vec4(light.ambient.rgb + light.diffuse.rgb, 1.0);
}
