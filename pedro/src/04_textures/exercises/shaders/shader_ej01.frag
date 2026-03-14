#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D tex1;
uniform sampler2D tex2;

void main()
{
    FragColor = mix(texture(tex1, vec2(TexCoord.x, -TexCoord.y)), texture(tex2, TexCoord), 0.2);
}
