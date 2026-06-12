#version 330 core

#define MAX_POS_LIGHTS 16
#define MAX_SPOT_LIGHTS 8

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_tex_coord;

out vec2 tex_coord;
out vec3 frag_pos;
out vec3 normal;
out vec3 dir_light_dir_view;
out vec3 pos_lights_pos_view[MAX_POS_LIGHTS];
out vec3 spot_lights_pos_view[MAX_SPOT_LIGHTS];
out vec3 spot_lights_dir_view[MAX_SPOT_LIGHTS];

struct directional_light_t {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct positional_light_t {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct spot_light_t {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutoff;
    float outer_cutoff;

    float constant;
    float linear;
    float quadratic;
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float normal_flip;
uniform directional_light_t dir_light;
uniform positional_light_t pos_lights[MAX_POS_LIGHTS];
uniform int pos_light_count;
uniform spot_light_t spot_lights[MAX_SPOT_LIGHTS];
uniform int spot_light_count;

void main() {
    mat4 vm = view * model;
    vec4 vm_pos = vm * vec4(a_pos, 1.);

    tex_coord = a_tex_coord;
    frag_pos = vec3(vm_pos);
    normal = mat3(transpose(inverse(vm))) * (a_normal * normal_flip);

    dir_light_dir_view = vec3(view * vec4(dir_light.direction, 0.));
    for (int i = 0; i < pos_light_count; ++i) {
        pos_lights_pos_view[i] = vec3(view * vec4(pos_lights[i].position, 1.));
    }
    for (int i = 0; i < spot_light_count; ++i) {
        spot_lights_pos_view[i] = vec3(view * vec4(spot_lights[i].position, 1.));
        spot_lights_dir_view[i] = vec3(view * vec4(spot_lights[i].direction, 0.));
    }

    gl_Position = projection * vm_pos;
}
