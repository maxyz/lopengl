#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aOffset;

out vec3 fColor;
uniform float lastIndex;

void main()
{
    vec2 pos = aPos * (gl_InstanceID / lastIndex);
    gl_Position = vec4(pos + aOffset, 0.0, 1.0);
    fColor = aColor - vec3(aOffset, 0.0);
} 
