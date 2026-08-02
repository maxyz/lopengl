#version 330 core
in vs_out_t {
    vec3 frag_pos;
    vec3 normal;
    vec2 tex_coords;
} fs_in;

out vec4 frag_color;

uniform sampler2D floor_texture;
uniform vec3 light_pos;
uniform vec3 view_pos;
uniform bool blinn;

void main() {           
    vec3 color = texture(floor_texture, fs_in.tex_coords).rgb;
    // ambient
    vec3 ambient = 0.05 * color;
    // diffuse
    vec3 light_dir = normalize(light_pos - fs_in.frag_pos);
    vec3 normal = normalize(fs_in.normal);
    float diff = max(dot(light_dir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 view_dir = normalize(view_pos - fs_in.frag_pos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = 0.0;
    if(blinn) {
        vec3 halfway_dir = normalize(light_dir + view_dir);  
        spec = pow(max(dot(normal, halfway_dir), 0.0), 32.0);
    } else {
        vec3 reflect_dir = reflect(-light_dir, normal);
        spec = pow(max(dot(view_dir, reflect_dir), 0.0), 8.0);
    }
    vec3 specular = vec3(0.3) * spec; // assuming bright white light color
    frag_color = vec4(ambient + diffuse + specular, 1.0);
}
