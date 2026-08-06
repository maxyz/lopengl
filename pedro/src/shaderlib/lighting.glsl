struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {    
    vec3 position; 

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic; 
};

struct Spotlight {
    vec3  position;
    vec3  direction;

    float cutOff;  
    float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

vec2 lightTexCoords;
vec3 lightNormal;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, sampler2D materialDiffuse, sampler2D materialSpecular, float materialShininess)
{
    // ambient
    vec3 ambient = light.ambient * vec3(texture(materialDiffuse, lightTexCoords));
  	
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(materialDiffuse, lightTexCoords));
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular = light.specular * spec * vec3(texture(materialSpecular, lightTexCoords));
    
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, sampler2D materialDiffuse, sampler2D materialSpecular, float materialShininess)
{
    vec3 ambient = light.ambient * vec3(texture(materialDiffuse, lightTexCoords));
  	
    // diffuse 
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(materialDiffuse, lightTexCoords));
    
    // specular
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
    vec3 specular = light.specular * spec * vec3(texture(materialSpecular, lightTexCoords));

    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
    	                light.quadratic * (distance * distance));

    return (ambient + (diffuse + specular) * attenuation);
}

vec3 CalcSpotlight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir, sampler2D materialDiffuse, sampler2D materialSpecular, float materialShininess)
{
    // ambient
    vec3 ambient = light.ambient * vec3(texture(materialDiffuse, lightTexCoords));

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    
    if(theta > light.outerCutOff) 
    {
        float epsilon   = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

        // diffuse 
        vec3 norm = normalize(lightNormal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = light.diffuse * diff * vec3(texture(materialDiffuse, lightTexCoords));
        
        // specular
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
        vec3 specular = light.specular * spec * vec3(texture(materialSpecular, lightTexCoords));

        float distance    = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + 
                            light.quadratic * (distance * distance));

        return (ambient + (diffuse + specular) * attenuation * intensity);

    } else {

        return ambient;

    } 
}