#version 330 core

in vec2 tex_coords;

out vec4 frag_color;

uniform sampler2D texture1;

float near = .01;
float far = .1;
float linearize_depth(float depth) {
  float z = depth * 2. - 1.;
  return (2. * near * far) / (far + near - z * (far - near));
}

void main() {
  float depth = clamp((far - linearize_depth(gl_FragCoord.z)) / far, 0., 1.);
  frag_color = vec4(vec3(depth), 1.) * texture(texture1, tex_coords);
}
