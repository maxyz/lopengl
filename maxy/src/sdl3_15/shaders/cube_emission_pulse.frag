#version 450

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos; // camera-relative world space
layout(location = 2) in vec3 frag_normal; // camera-relative world space

layout(set = 2, binding = 0) uniform sampler2D diffuse_tex;
layout(set = 2, binding = 1) uniform sampler2D specular_tex;
layout(set = 2, binding = 2) uniform sampler2D emission_tex;

// .x = shininess, .y = time (seconds); .zw unused padding
layout(set = 3, binding = 0) uniform MaterialBlock {
    float shininess;
    float time;
} material;

layout(set = 3, binding = 1) uniform LightBlock {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 position; // camera-relative world space
} light;

layout(location = 0) out vec4 frag_color;

// Two-peak Gaussian model of a heartbeat at 72 bpm.
// phase in [0,1): lub at ~0.12, dub at ~0.32, silent for the rest of the beat.
float heartbeat_pulse(float time) {
    float phase = fract(time * 1.2); // 72 bpm = 1.2 beats per second
    float lub = exp(-pow((phase - 0.12) * 30.0, 2.0));
    float dub = exp(-pow((phase - 0.32) * 40.0, 2.0)) * 0.6;
    return lub + dub;
}

void main() {
    vec3 diffuse_color = texture(diffuse_tex, frag_tex_coord).rgb;
    vec3 specular_color = texture(specular_tex, frag_tex_coord).rgb;
    vec3 emission_color = texture(emission_tex, frag_tex_coord + vec2(0.0, material.time)).rgb
            * floor(vec3(1.f) - texture(specular_tex, frag_tex_coord).rgb);

    vec3 ambient = light.ambient.rgb * diffuse_color;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light.position.xyz - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse.rgb * diff * diffuse_color;

    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular.rgb * spec * specular_color;

    vec3 emission = emission_color * (1. - heartbeat_pulse(material.time));

    frag_color = vec4(ambient + diffuse + specular + emission, 1.0);
}
