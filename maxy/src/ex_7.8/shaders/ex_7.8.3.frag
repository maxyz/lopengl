#version 330 core

out vec4 FragColor;

in vec3 ourColor;
in vec2 texCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
  vec2 coord = vec2(1.0 - texCoord.x, texCoord.y);
  FragColor = mix(texture(texture1, texCoord), texture(texture2, coord * 2), 0.2);
}
