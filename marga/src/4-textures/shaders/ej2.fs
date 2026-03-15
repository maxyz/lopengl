#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float scale;
vec2 ScaleCoord;

void main()
{
    ScaleCoord = vec2(scale*TexCoord.x, scale*TexCoord.y);
    FragColor = mix(texture(texture1, ScaleCoord), texture(texture2, ScaleCoord), 0.2);
}
