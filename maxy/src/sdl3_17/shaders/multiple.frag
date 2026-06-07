#version 450

#define NUM_POS_LIGHTS 4
#define NUM_SPOT_LIGHTS 2

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos; // camera-relative world space
layout(location = 2) in vec3 frag_normal; // camera-relative world space

layout(set = 2, binding = 0) uniform sampler2D diffuse_tex;
layout(set = 2, binding = 1) uniform sampler2D specular_tex;

layout(set = 3, binding = 0) uniform MaterialBlock {
    float shininess;
} material;

layout(set = 3, binding = 1) uniform DirectionalLightBlock {
    vec4 direction; // world-space; negated below to get toward-light direction
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
} dir_light;

struct pos_light_t {
    vec4 position; // camera-relative world space
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    float pad;
};

layout(set = 3, binding = 2) uniform PosLightsBlock {
    pos_light_t lights[NUM_POS_LIGHTS];
} pos_lights;

struct spot_light_t {
    vec4 position; // camera-relative world space
    vec4 direction; // world-space cone axis; negated below
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float cutoff; // cos(inner_angle)
    float outer_cutoff; // cos(outer_angle)
    float constant;
    float linear;
    float quadratic;
};

layout(set = 3, binding = 3) uniform SpotLightsBlock {
    spot_light_t lights[NUM_SPOT_LIGHTS];
} spot_lights;

layout(location = 0) out vec4 frag_color;

vec3 directional_contribution(vec3 norm, vec3 view_dir, vec3 diffuse_color, vec3 specular_color) {
    vec3 light_dir = normalize(-dir_light.direction.xyz);
    vec3 ambient = dir_light.ambient.rgb * diffuse_color;

    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = dir_light.diffuse.rgb * diff * diffuse_color;

    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = dir_light.specular.rgb * spec * specular_color;

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
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
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
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular.rgb * spec * specular_color;

    // Ambient is not gated by intensity: surfaces outside the cone still see ambient.
    return attenuation * (ambient + intensity * (diffuse + specular));
}

void main() {
    vec3 diffuse_color = texture(diffuse_tex, frag_tex_coord).rgb;
    vec3 specular_color = texture(specular_tex, frag_tex_coord).rgb;
    vec3 norm = normalize(frag_normal);
    vec3 view_dir = normalize(-frag_pos);

    vec3 result = directional_contribution(norm, view_dir, diffuse_color, specular_color);

    for (int i = 0; i < NUM_POS_LIGHTS; ++i) {
        result += positional_contribution(
                pos_lights.lights[i], norm, view_dir, diffuse_color, specular_color
            );
    }
    for (int i = 0; i < NUM_SPOT_LIGHTS; ++i) {
        result += spot_contribution(
                spot_lights.lights[i], norm, view_dir, diffuse_color, specular_color
            );
    }

    frag_color = vec4(result, 1.0);
}
