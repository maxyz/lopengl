#version 330 core

#define NR_POS_LIGHTS 4
#define NR_SPOT_LIGHTS 2

out vec4 frag_color;

in vec2 tex_coord;
in vec3 frag_pos;
in vec3 normal;
in vec3 dir_light_dir_view;
in vec3 pos_lights_pos_view[NR_POS_LIGHTS];
in vec3 spot_lights_pos_view[NR_SPOT_LIGHTS];
in vec3 spot_lights_dir_view[NR_SPOT_LIGHTS];

struct material_t {
  sampler2D diffuse;
  sampler2D specular;

  float shininess;
};

struct directional_light_t {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct directional_light_view_t {
  vec3 direction_view;
  directional_light_t light;
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

struct positional_light_view_t {
  vec3 position_view;
  positional_light_t light;
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
struct spot_light_view_t {
  vec3 position_view;
  vec3 direction_view;
  spot_light_t light;
};

uniform material_t material;
uniform directional_light_t dir_light;
uniform positional_light_t pos_lights[NR_POS_LIGHTS];
uniform spot_light_t spot_lights[NR_SPOT_LIGHTS];

vec3 directional_light_process(directional_light_view_t light_view, vec3 normal, vec3 view_dir) {
  vec3 ambient = light_view.light.ambient * texture(material.diffuse, tex_coord).rgb;

  vec3 light_dir = normalize(-light_view.direction_view);

  float diff = max(dot(normal, light_dir), 0.0);
  vec3 diffuse = light_view.light.diffuse * diff * texture(material.diffuse, tex_coord).rgb;

  vec3 reflect_dir = reflect(-light_dir, normal);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light_view.light.specular * spec * texture(material.specular, tex_coord).rgb;

  return ambient + diffuse + specular;
}

vec3 positional_light_process(positional_light_view_t light_view, vec3 normal, vec3 view_dir, vec3 frag_pos) {
  vec3 ambient = light_view.light.ambient * texture(material.diffuse, tex_coord).rgb;
  vec3 light_pos_from_frag = light_view.position_view - frag_pos;

  vec3 light_dir = normalize(light_pos_from_frag);

  float diff = max(dot(normal, light_dir), 0.0);
  vec3 diffuse = light_view.light.diffuse * diff * texture(material.diffuse, tex_coord).rgb;

  vec3 reflect_dir = reflect(-light_dir, normal);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light_view.light.specular * spec * texture(material.specular, tex_coord).rgb;

  float distance = length(light_pos_from_frag);
  float attenuation = 1.0 / (light_view.light.constant + light_view.light.linear * distance + light_view.light.quadratic * distance * distance);

  return attenuation * (ambient + diffuse + specular);
}

vec3 spot_light_process(spot_light_view_t light_view, vec3 normal, vec3 view_dir, vec3 frag_pos) {
  vec3 ambient = light_view.light.ambient * texture(material.diffuse, tex_coord).rgb;
  vec3 light_pos_from_frag = light_view.position_view - frag_pos;

  vec3 light_dir = normalize(light_pos_from_frag);
  float theta = dot(light_dir, normalize(-light_view.direction_view));
  float epsilon = light_view.light.cutoff - light_view.light.outer_cutoff;
  float intensity = clamp((theta - light_view.light.outer_cutoff) / epsilon, 0., 1.);

  float diff = max(dot(normal, light_dir), 0.0);
  vec3 diffuse = light_view.light.diffuse * diff * texture(material.diffuse, tex_coord).rgb;

  vec3 reflect_dir = reflect(-light_dir, normal);
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
  vec3 specular = light_view.light.specular * spec * texture(material.specular, tex_coord).rgb;

  float distance = length(light_pos_from_frag);
  float attenuation = 1.0 / (light_view.light.constant + light_view.light.linear * distance + light_view.light.quadratic * distance * distance);

  return attenuation * (ambient + (intensity * (diffuse + specular)));
}

void main() {
  directional_light_view_t dir_light_view;
  dir_light_view.direction_view = dir_light_dir_view;
  dir_light_view.light = dir_light;

  positional_light_view_t pos_lights_view[NR_POS_LIGHTS];
  for (int i=0; i < NR_POS_LIGHTS; ++i) {
    pos_lights_view[i].position_view = pos_lights_pos_view[i];
    pos_lights_view[i].light = pos_lights[i];
  }
  spot_light_view_t spot_lights_view[NR_SPOT_LIGHTS];
  for (int i=0; i < NR_SPOT_LIGHTS; ++i) {
    spot_lights_view[i].position_view = spot_lights_pos_view[i];
    spot_lights_view[i].direction_view = spot_lights_dir_view[i];
    spot_lights_view[i].light = spot_lights[i];
  }
  vec3 norm = normalize(normal);
  vec3 view_dir = normalize(-frag_pos);

  vec3 result = directional_light_process(dir_light_view, norm, view_dir);
  for (int i = 0; i < NR_POS_LIGHTS; ++i) {
    result += positional_light_process(pos_lights_view[i], normal, view_dir, frag_pos);
  }
  for (int i = 0; i < NR_SPOT_LIGHTS; ++i) {
    result += spot_light_process(spot_lights_view[i], normal, view_dir, frag_pos);
  }
  frag_color = vec4(result, 1.);
}
