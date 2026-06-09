#version 460 core

layout(location = 0) in vec2 tex_coords;

layout(set = 2, binding = 0) uniform sampler2D texture1;

layout(set = 3, binding = 0) uniform DepthRangeBlock {
    float near;
    float far;
};

layout(location = 0) out vec4 frag_color;

float linearize_depth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    float linear_depth = linearize_depth(gl_FragCoord.z);
    float brightness = clamp((far - linear_depth) / far, 0.0, 1.0);
    frag_color = vec4(vec3(brightness), 1.0) * texture(texture1, tex_coords);
}
