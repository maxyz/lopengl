#version 460 core

#define MAX_POS_LIGHTS  16
#define MAX_SPOT_LIGHTS 8

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos;
layout(location = 2) in vec3 frag_normal;

layout(set = 2, binding = 0) uniform sampler2D diffuse_tex;
layout(set = 2, binding = 1) uniform sampler2D specular_tex;

layout(set = 3, binding = 0) uniform SceneParamsBlock {
    float shininess;
    int pos_count;
    int spot_count;
    int pad;
    vec4 dir_direction;
    vec4 dir_ambient;
    vec4 dir_diffuse;
    vec4 dir_specular;
} scene;

struct pos_light_t {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    float pad;
};

layout(set = 3, binding = 1) uniform PosLightsBlock {
    pos_light_t lights[MAX_POS_LIGHTS];
} pos_lights;

struct spot_light_t {
    vec4 position;
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float cutoff;
    float outer_cutoff;
    float constant;
    float linear;
    float quadratic;
    // std140 rounds this struct to 112 bytes implicitly
};

layout(set = 3, binding = 2) uniform SpotLightsBlock {
    spot_light_t lights[MAX_SPOT_LIGHTS];
} spot_lights;

layout(set = 3, binding = 3) uniform FlashlightBlock {
    vec4 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float cutoff;
    float outer_cutoff;
    float constant;
    float linear;
    float quadratic;
    // std140 rounds block to 96 bytes implicitly
} flashlight;

layout(location = 0) out vec4 frag_color;

vec3 directional_contribution(vec3 norm, vec3 view_dir, vec3 diffuse_color, vec3 specular_color) {
    vec3 light_dir = normalize(-scene.dir_direction.xyz);
    vec3 ambient = scene.dir_ambient.rgb * diffuse_color;
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = scene.dir_diffuse.rgb * diff * diffuse_color;
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), scene.shininess);
    vec3 specular = scene.dir_specular.rgb * spec * specular_color;
    return ambient + diffuse + specular;
}

vec3 positional_contribution(
    pos_light_t light, vec3 norm, vec3 view_dir, vec3 diffuse_color, vec3 specular_color
) {
    vec3 to_light = light.position.xyz - frag_pos;
    float dist = length(to_light);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
    vec3 light_dir = normalize(to_light);
    vec3 ambient = light.ambient.rgb * diffuse_color;
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse.rgb * diff * diffuse_color;
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), scene.shininess);
    vec3 specular = light.specular.rgb * spec * specular_color;
    return attenuation * (ambient + diffuse + specular);
}

vec3 spot_contribution(
    spot_light_t light, vec3 norm, vec3 view_dir, vec3 diffuse_color, vec3 specular_color
) {
    vec3 to_light = light.position.xyz - frag_pos;
    float dist = length(to_light);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
    vec3 light_dir = normalize(to_light);
    float theta = dot(light_dir, normalize(-light.direction.xyz));
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);
    vec3 ambient = light.ambient.rgb * diffuse_color;
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse.rgb * diff * diffuse_color;
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), scene.shininess);
    vec3 specular = light.specular.rgb * spec * specular_color;
    return attenuation * (ambient + intensity * (diffuse + specular));
}

vec3 flashlight_contribution(vec3 norm, vec3 view_dir, vec3 diffuse_color, vec3 specular_color) {
    vec3 to_light = -frag_pos;
    float dist = length(to_light);
    float attenuation = 1.0 / (flashlight.constant + flashlight.linear * dist + flashlight.quadratic * dist * dist);
    vec3 light_dir = normalize(to_light);
    float theta = dot(light_dir, normalize(-flashlight.direction.xyz));
    float epsilon = flashlight.cutoff - flashlight.outer_cutoff;
    float intensity = clamp((theta - flashlight.outer_cutoff) / epsilon, 0.0, 1.0);
    vec3 ambient = flashlight.ambient.rgb * diffuse_color;
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = flashlight.diffuse.rgb * diff * diffuse_color;
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), scene.shininess);
    vec3 specular = flashlight.specular.rgb * spec * specular_color;
    return attenuation * (ambient + intensity * (diffuse + specular));
}

void main() {
    vec4 diffuse_texel = texture(diffuse_tex, frag_tex_coord);
    if (diffuse_texel.a < 0.1) discard;
    vec3 diffuse_color = diffuse_texel.rgb;
    vec3 specular_color = texture(specular_tex, frag_tex_coord).rgb;
    vec3 norm = normalize(frag_normal);
    vec3 view_dir = normalize(-frag_pos);

    vec3 result = directional_contribution(norm, view_dir, diffuse_color, specular_color);
    for (int i = 0; i < scene.pos_count; ++i)
        result += positional_contribution(pos_lights.lights[i], norm, view_dir, diffuse_color, specular_color);
    for (int i = 0; i < scene.spot_count; ++i)
        result += spot_contribution(spot_lights.lights[i], norm, view_dir, diffuse_color, specular_color);
    result += flashlight_contribution(norm, view_dir, diffuse_color, specular_color);

    // Preserve alpha from diffuse texture so blending works for semi-transparent surfaces.
    frag_color = vec4(result, diffuse_texel.a);
}
