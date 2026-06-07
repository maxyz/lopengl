#version 450

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos; // camera-relative world space
layout(location = 2) in vec3 frag_normal; // camera-relative world space

// Each sampler2D field from the OpenGL struct becomes a separate top-level binding.
// fragment_samplers=2 in pipeline_desc_t; draw() binds them in index order.
layout(set = 2, binding = 0) uniform sampler2D diffuse_tex;
layout(set = 2, binding = 1) uniform sampler2D specular_tex;

// Only shininess remains as a flat material value; diffuse and specular come from textures.
// Pushed as vec4 (.x = shininess) to satisfy the 16-byte minimum uniform buffer size.
layout(set = 3, binding = 0) uniform MaterialBlock {
    float shininess;
} material;

layout(set = 3, binding = 1) uniform LightBlock {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position; // camera-relative world space
} light;

layout(location = 0) out vec4 frag_color;

void main() {
    vec3 diffuse_color = texture(diffuse_tex, frag_tex_coord).rgb;
    vec3 specular_color = texture(specular_tex, frag_tex_coord).rgb;

    vec3 ambient = light.ambient.rgb * diffuse_color;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light.position.xyz - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse.rgb * diff * diffuse_color;

    vec3 view_dir = normalize(-frag_pos); // camera at origin in camera-relative space
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular.rgb * spec * specular_color;

    frag_color = vec4(ambient + diffuse + specular, 1.0);
}
