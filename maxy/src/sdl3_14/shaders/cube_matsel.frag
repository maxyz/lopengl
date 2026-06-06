#version 450

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos; // camera-relative world space
layout(location = 2) in vec3 frag_normal; // camera-relative world space

layout(set = 2, binding = 0) uniform sampler2D tex1;
layout(set = 2, binding = 1) uniform sampler2D tex2;

layout(set = 3, binding = 0) uniform MaterialBlock {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular_shininess; // .rgb = specular, .w = shininess
} material;

layout(set = 3, binding = 1) uniform LightBlock {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position; // camera-relative world space
} light;

// Pushed once per draw as fragment uniform slot 2. use_texture=0 binds a
// neutral (1,1,1) color instead of sampling, showing pure material reflectance.
// Drivers fold a uniform-driven branch into a select, not per-fragment divergence.
layout(set = 3, binding = 2) uniform FlagsBlock {
    uint use_texture;
    uint pad0;
    uint pad1;
    uint pad2;
} flags;

layout(location = 0) out vec4 frag_color;

void main() {
    vec3 ambient = light.ambient.rgb * material.ambient.rgb;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light.position.xyz - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse.rgb * (diff * material.diffuse.rgb);

    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.specular_shininess.w);
    vec3 specular = light.specular.rgb * (spec * material.specular_shininess.rgb);

    vec4 tex_color = flags.use_texture != 0u
        ? mix(texture(tex1, frag_tex_coord), texture(tex2, frag_tex_coord), 0.2) : vec4(1.0);
    frag_color = vec4((ambient + diffuse) * tex_color.rgb + specular, 1.0);
}
