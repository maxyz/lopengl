#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

#include "../shaderlib/lighting.glsl"

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float     shininess;
};

uniform Material material;

uniform DirLight dirLight;

#define NR_POINT_LIGHTS 4  
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform Spotlight spotlight;

uniform vec3 viewPos;

void main()
{
    lightTexCoords = TexCoords;
    lightNormal = Normal;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);
    
    result = CalcDirLight(dirLight, Normal, viewDir,
             material.diffuse,
             material.specular,
             material.shininess
    );

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
  	    result += CalcPointLight(pointLights[i], Normal, FragPos, viewDir,
                  material.diffuse,
                  material.specular,
                  material.shininess
        );

    //result += CalcSpotlight(spotlight, Normal, FragPos, viewDir);

    // emission
    // vec3 emission = vec3(texture(material.emission, TexCoords));
    // result += emission;

    FragColor = vec4(result, 1.0);
}