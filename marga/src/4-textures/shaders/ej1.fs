#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

vec2 InvCoord;

void main()
{
    InvCoord = vec2(-1*TexCoord.x, -1*TexCoord.y);
    FragColor = mix(texture(texture1, TexCoord), texture(texture2, InvCoord), 0.2);
}
