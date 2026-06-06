#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

layout(set = 1, binding = 0) uniform Model {
    mat4 model;
};
layout(set = 1, binding = 1) uniform View {
    mat4 view;
};
layout(set = 1, binding = 2) uniform Projection {
    mat4 projection;
};
// Light position in world space; transformed to view space here.
layout(set = 1, binding = 3) uniform LightPos {
    vec4 light_pos;
};
layout(set = 1, binding = 4) uniform VertexLighting {
    vec4 light_color;
    vec4 light_strengths; // x=ambient, y=diffuse, z=specular, w=shininess
};

layout(location = 0) out vec2 frag_tex_coord;
// Full Phong intensity computed per-vertex; rasterizer interpolates it.
layout(location = 1) out vec3 light_gouraud;

void main() {
    mat4 view_model = view * model;
    gl_Position = projection * view_model * vec4(position, 1.0);

    vec3 frag_pos = vec3(view_model * vec4(position, 1.0));
    vec3 frag_normal = mat3(transpose(inverse(view_model))) * normal;
    vec3 light_pos_view = vec3(view * vec4(light_pos.xyz, 1.0));

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

    frag_tex_coord = tex_coord;
    light_gouraud = ambient + diffuse + specular;
}
