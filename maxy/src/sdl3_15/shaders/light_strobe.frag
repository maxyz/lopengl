#version 450

// Only ambient and diffuse are read; the rest of light_uniforms_t (specular,
// position) is pushed but ignored. Pushing more bytes than declared is safe.
layout(set = 3, binding = 0) uniform LightBlock {
    vec4 ambient;
    vec4 diffuse;
} light;

layout(location = 0) out vec4 frag_color;

void main() {
    // ambient + diffuse makes the source cube appear over-bright relative to
    // the surfaces it illuminates, reinforcing that it is the emitter.
    frag_color = vec4((light.ambient + light.diffuse).rgb, 1.0);
}
