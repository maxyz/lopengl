#version 450

layout(location = 0) in vec3 position;

layout(set = 1, binding = 0) uniform Rotation {
    float angle;
    float aspect; // width / height -- divide x to maintain correct proportions
};

void main() {
    float c = cos(angle);
    float s = sin(angle);
    // Rotate in square world space, then apply aspect correction to clip space.
    gl_Position = vec4(
        (position.x * c - position.y * s) / aspect,
         position.x * s + position.y * c,
         position.z,
         1.0
    );
}
