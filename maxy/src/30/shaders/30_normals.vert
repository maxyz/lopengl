#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

out vs_out_t {
    vec3 normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main() {
    mat3 normal_matrix = mat3(transpose(inverse(view * model)));
    vs_out.normal = vec3(vec4(normal_matrix * a_normal, 0.0));
    gl_Position = view * model * vec4(a_pos, 1.0); 
}


