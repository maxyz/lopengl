#version 330 core

in vec2 TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float     shininess;
};
uniform Material material;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light; 
uniform float time;
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
    // Calculates angles of reflection and view for specular light
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    // Calculate the specular component
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    //vec3 emission = vec3(texture(material.emission, TexCoords));

    // Emission Masked
    vec3 emission = texture(material.emission,TexCoords + vec2(0.0,time)).rgb * floor(vec3(1.f) - texture(material.specular,TexCoords ).rgb);

    vec3 result = ambient + diffuse + specular + emission;
    FragColor = vec4(result, 1.0);
}
