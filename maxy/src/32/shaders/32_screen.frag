#version 330 core
in vec2 tex_coords;

out vec4 frag_color;

uniform sampler2D screen_texture;

void main() {
    vec3 col = texture(screen_texture, tex_coords).rgb;
    float grayscale = 0.2126 * col.r + 0.7152 * col.g + 0.0722 * col.b;
    frag_color = vec4(vec3(grayscale), 1.0);
} 
