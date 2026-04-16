#version 330 core

out vec4 frag_color;

in vec2 tex_coord;
in vec3 light_gouraud;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 object_color;

void main() {
  frag_color = vec4(light_gouraud * object_color, 1.0) * mix(texture(texture1, tex_coord), texture(texture2, tex_coord), 0.2);
}
