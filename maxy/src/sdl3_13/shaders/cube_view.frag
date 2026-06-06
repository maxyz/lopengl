#version 450

layout(location = 0) in vec2 frag_tex_coord;
layout(location = 1) in vec3 frag_pos; // view space
layout(location = 2) in vec3 frag_normal; // view space
layout(location = 3) in vec3 light_pos_view; // view space

layout(set = 2, binding = 0) uniform sampler2D tex1;
layout(set = 2, binding = 1) uniform sampler2D tex2;

layout(set = 3, binding = 0) uniform Lighting {
    vec4 object_color;
    vec4 light_color;
    vec4 light_strengths; // x=ambient, y=diffuse, z=specular, w=shininess
};

layout(location = 0) out vec4 frag_color;

void main() {
    vec3 ambient = light_strengths.x * light_color.rgb;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos_view - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light_strengths.y * diff * light_color.rgb;

    // In view space the camera is at the origin, so view_dir is simply -frag_pos.
    vec3 view_dir = normalize(-frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), light_strengths.w);
    vec3 specular = light_strengths.z * spec * light_color.rgb;

    vec3 intensity = ambient + diffuse + specular;
    vec4 tex = mix(texture(tex1, frag_tex_coord), texture(tex2, frag_tex_coord), 0.2);
    frag_color = vec4(intensity * object_color.rgb, 1.0) * tex;
}
