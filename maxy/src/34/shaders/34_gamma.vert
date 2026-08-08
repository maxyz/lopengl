#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_tex_coords;

out vs_out_t {
    vec3 frag_pos;
    vec3 normal;
    vec2 tex_coords;
} vs_out;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    vs_out.frag_pos = a_pos;
    vs_out.normal = a_normal;
    vs_out.tex_coords = a_tex_coords;
    gl_Position = projection * view * vec4(a_pos, 1.0);
}
