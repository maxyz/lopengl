#version 330 core
layout (points) in;
layout (points, max_vertices = 1) out;

out vec3 fColor;

in VS_OUT 
{
    vec3 color;
} gs_in[];

void main() {    
    fColor = gs_in[0].color;
    gl_Position = gl_in[0].gl_Position; 
    EmitVertex();
    EndPrimitive();
}  