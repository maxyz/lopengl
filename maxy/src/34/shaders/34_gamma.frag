#version 330 core
in vs_out_t {
    vec3 frag_pos;
    vec3 normal;
    vec2 tex_coords;
} fs_in;

struct positional_light_t {
  vec3 position;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

out vec4 frag_color;

uniform sampler2D floor_texture;

uniform vec3 view_pos;
uniform bool gamma;

#define NUM_POS_LIGHTS 4
uniform positional_light_t[NUM_POS_LIGHTS] positional_lights;

vec3 positional_light_process(positional_light_t light, vec3 norm, vec3 frag_pos, vec3 diffuse_rgb) {
  vec3 ambient = light.ambient * diffuse_rgb;

  vec3 light_pos_from_frag = light.position - frag_pos;

  vec3 light_dir = normalize(light_pos_from_frag);

  float diff = max(dot(norm, light_dir), 0.0);
  vec3 diffuse = light.diffuse * diff * diffuse_rgb;

  vec3 view_dir = normalize(view_pos - frag_pos);
  vec3 halfway_dir = normalize(light_dir + view_dir);

  // float spec = pow(max(dot(norm, halfway_dir), 0.0), material.shininess);
  float spec = pow(max(dot(norm, halfway_dir), 0.0), 16.0);
  vec3 specular = light.specular * spec;
  // * texture(material.specular, tex_coord).rgb;

  float distance = length(light_pos_from_frag);
  // float attenuation = 1.0 / (light_view.light.constant + light_view.light.linear * distance + light_view.light.quadratic * distance * distance);
  float constant = 0.0, linear = 1.0, quadratic = 0.0;
  if (gamma) {
      quadratic = 1.0;
      linear = 0.0;
  }
  float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);

  return attenuation * (ambient + diffuse + specular);
}


void main() {
    vec3 color = texture(floor_texture, fs_in.tex_coords).rgb;
    vec3 lighting = vec3(0.0);
    for (int i = 0; i < NUM_POS_LIGHTS; ++i) {
        lighting += positional_light_process(positional_lights[i], fs_in.normal, fs_in.frag_pos, vec3(1.0));
    }
    color *= lighting;
    if (gamma) {
        color = pow(color, vec3(1.0/2.2));
    }
    frag_color = vec4(color, 1.0);
}
