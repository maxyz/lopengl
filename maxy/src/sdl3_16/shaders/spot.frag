#version 450

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos; // camera-relative world space
layout(location = 2) in vec3 frag_normal; // camera-relative world space

layout(set = 2, binding = 0) uniform sampler2D diffuse_tex;
layout(set = 2, binding = 1) uniform sampler2D specular_tex;

layout(set = 3, binding = 0) uniform MaterialBlock {
    float shininess;
} material;

layout(set = 3, binding = 1) uniform SpotLightBlock {
    vec4 position; // camera-relative world space
    vec4 direction; // world-space cone axis; negated below to get toward-light direction
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float cutoff; // cos(inner_angle): fully lit inside this cone
    float outer_cutoff; // cos(outer_angle): fully dark outside this cone
    float constant;
    float linear;
    float quadratic;
} light;

layout(location = 0) out vec4 frag_color;

void main() {
    vec3 diffuse_color = texture(diffuse_tex, frag_tex_coord).rgb;
    vec3 specular_color = texture(specular_tex, frag_tex_coord).rgb;

    vec3 ambient = light.ambient.rgb * diffuse_color;

    // theta: cosine of angle between toward-fragment and cone axis.
    // intensity: 1 inside inner cone, 0 outside outer cone, smooth in between.
    vec3 light_dir = normalize(light.position.xyz - frag_pos);
    float theta = dot(light_dir, normalize(-light.direction.xyz));
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    vec3 norm = normalize(frag_normal);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse.rgb * diff * diffuse_color;

    vec3 view_dir = normalize(-frag_pos); // camera at origin in camera-relative space
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular.rgb * spec * specular_color;

    float dist = length(light.position.xyz - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);

    // Ambient is not gated by intensity: surfaces outside the cone still see ambient light.
    frag_color = vec4(attenuation * (ambient + intensity * (diffuse + specular)), 1.0);
}
