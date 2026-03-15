#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float scale;
vec2 ScaleCoord;
float offset;

void main()
{
    offset = (1 - scale)/2;
    ScaleCoord = vec2(TexCoord.x*scale + offset, TexCoord.y*scale+offset);
    FragColor = mix(texture(texture1, ScaleCoord), texture(texture2, ScaleCoord), 0.2);
}
