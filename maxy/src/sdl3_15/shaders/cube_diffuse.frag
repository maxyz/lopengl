#version 450

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos; // camera-relative world space
layout(location = 2) in vec3 frag_normal; // camera-relative world space

// In OpenGL, sampler2D lived inside a uniform struct (diffuse_map_t.diffuse).
// SPIR-V forbids opaque types inside uniform blocks: samplers must be top-level
// descriptor bindings. The scalar fields (specular, shininess) move to MaterialBlock.
layout(set = 2, binding = 0) uniform sampler2D diffuse_tex;

layout(set = 3, binding = 0) uniform MaterialBlock {
    vec4 specular_shininess; // .rgb = specular color, .w = shininess
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

    vec3 ambient = light.ambient.rgb * diffuse_color;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light.position.xyz - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse.rgb * diff * diffuse_color;

    vec3 view_dir = normalize(-frag_pos); // camera at origin in camera-relative space
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.specular_shininess.w);
    vec3 specular = light.specular.rgb * spec * material.specular_shininess.rgb;

    frag_color = vec4(ambient + diffuse + specular, 1.0);
}
