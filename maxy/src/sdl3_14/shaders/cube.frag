#version 450

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos; // camera-relative world space
layout(location = 2) in vec3 frag_normal; // camera-relative world space

layout(set = 2, binding = 0) uniform sampler2D tex1;
layout(set = 2, binding = 1) uniform sampler2D tex2;

// Block instance names are needed because both blocks share member names
// (ambient, diffuse, specular). This is the struct-like access pattern from
// the LearnOpenGL materials chapter.
layout(set = 3, binding = 0) uniform MaterialBlock {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular_shininess; // .rgb = specular, .w = shininess
} material;

layout(set = 3, binding = 1) uniform LightBlock {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position; // camera-relative world space; no view_pos needed
} light;

layout(location = 0) out vec4 frag_color;

void main() {
    vec3 ambient = light.ambient.rgb * material.ambient.rgb;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light.position.xyz - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse.rgb * (diff * material.diffuse.rgb);

    // Camera is at the origin in camera-relative world space.
    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.specular_shininess.w);
    vec3 specular = light.specular.rgb * (spec * material.specular_shininess.rgb);

    vec4 tex_color = mix(texture(tex1, frag_tex_coord), texture(tex2, frag_tex_coord), 0.2);
    // Texture modulates ambient+diffuse; specular is from the light, not the surface colour.
    frag_color = vec4((ambient + diffuse) * tex_color.rgb + specular, 1.0);
}
