#version 330 core

in vec2 TexCoords;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float     shininess;
};
uniform Material material;


// Directional light (aka the sun)
struct DirLight {
    bool is_active;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform DirLight dirLight;
// Returns the values after applying directional light
vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);  

// Positional light, with attenuation, there can be many of these
struct PointLight {
    bool is_active;
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir); 

struct SpotLight {
    bool is_active;
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};
uniform SpotLight spotLight; 
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir); 

uniform vec3 viewPos;

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;
  
void main()
{
  vec4 texColor = texture(material.texture_diffuse1, TexCoords);
/*  if(texColor.a < 0.1)
    discard;*/
  // define an output color value
  vec4 outputColor = vec4(0.0);
  // General parameters needed for the lights
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);
  // add the directional light's contribution to the output
  outputColor += CalcDirLight(dirLight, norm, viewDir);
  // do the same for all point lights
  for(int i = 0; i < NR_POINT_LIGHTS; i++) {
  	outputColor += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
  }
  // and add others lights as well (like spotlights)
  outputColor += CalcSpotLight(spotLight, norm, FragPos, viewDir);
  
  FragColor = outputColor;
}  

// Directional light calculation
vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    if (!light.is_active) return vec4(0.0);
    vec3 lightDir = normalize(light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec4 ambient  = vec4(light.ambient, 1.0)  * texture(material.texture_diffuse1, TexCoords);
    vec4 diffuse  = vec4(light.diffuse, 1.0)  * diff * texture(material.texture_diffuse1, TexCoords);
    vec4 specular = vec4(light.specular, 1.0) * spec * texture(material.texture_specular1, TexCoords);
    return (ambient + diffuse + specular);
} 

// Positional light calculation, with attenuation
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    if (!light.is_active) return vec4(0.0);
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
  			     light.quadratic * (distance * distance));
    // combine results
    vec4 ambient  = vec4(light.ambient, 1.0)  * texture(material.texture_diffuse1, TexCoords);
    vec4 diffuse  = vec4(light.diffuse, 1.0)  * diff * texture(material.texture_diffuse1, TexCoords);
    vec4 specular = vec4(light.specular, 1.0) * spec * texture(material.texture_specular1, TexCoords);
    return (ambient + diffuse + specular) * attenuation;
}

// Spotlight calculation
vec4 CalcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir)
{
    if (!light.is_active) return vec4(0.0);
    vec3 lightDir = normalize(light.position - FragPos);
    // Calculate the impact of the light
    float diff = max(dot(norm, lightDir), 0.0);
    // Calculates angles of reflection and view for specular light
    vec3 reflectDir = reflect(-lightDir, norm);
    // Calculate the specular component
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec4 ambient  = vec4(light.ambient, 1.0)  * texture(material.texture_diffuse1, TexCoords);
    vec4 diffuse  = vec4(light.diffuse, 1.0)  * diff * texture(material.texture_diffuse1, TexCoords);
    vec4 specular = vec4(light.specular, 1.0) * spec * texture(material.texture_specular1, TexCoords);
   
    // Attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                light.quadratic * (distance * distance));

    // Cut off
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    return (ambient + (diffuse + specular)*intensity) * attenuation;
}


