#version 330 core

struct Material {
    sampler2D diffuse;
    vec3      specular;
    float     shininess;
};
in vec2 TexCoords;

uniform Material material;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light; 

uniform vec3 viewPos;

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;
  
void main()
{
    // Calculate the angle of the light, for diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    // Calculate the impact of the light
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    // Ambient set based on diffuse
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));


    // Specular light
    // Calculates angles of reflection and view
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    // Calculate the specular component
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
