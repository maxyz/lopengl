#version 330 core

in vec2 tex_coords;

out vec4 frag_color;

uniform sampler2D texture1;

void main() {
  vec4 tex_color = texture(texture1, tex_coords);
  if (tex_color.a < 0.1) {
    discard;
  }
  frag_color = tex_color;
}
