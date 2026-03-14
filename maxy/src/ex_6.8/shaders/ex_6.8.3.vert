#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

uniform vec2 offset = vec2(0.0, 0.0);

out vec3 ourColor;

void main() {
  gl_Position = vec4(offset.x + aPos.x, offset.y + aPos.y, aPos.z, 1.0);
  ourColor = vec3(offset.x + aPos.x, offset.y + aPos.y, aPos.z);
}
