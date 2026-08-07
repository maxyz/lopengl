#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

#include "../shaderlib/lighting.glsl"

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

#define NR_POINT_LIGHTS 1

uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 viewPos;

void main()
{
    lightTexCoords = fs_in.TexCoords;
    lightNormal = fs_in.Normal;

    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 result = vec3(0.0);
    
    result = CalcDirLight(dirLight, viewDir,
             material.diffuse,
             material.specular,
             material.shininess
    );

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
  	    result += CalcPointLight(pointLights[i], fs_in.FragPos, viewDir,
                  material.diffuse,
                  material.specular,
                  material.shininess
        );

    //result += CalcSpotlight(spotlight, fs_in.FragPos, viewDir);

    FragColor = vec4(result, 1.0);
}