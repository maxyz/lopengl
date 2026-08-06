#include "lighting.hpp"

void DirectionaLight::sendToShader(Shader& shader)
{
    shader.setVec3("DirLight.ambient"  , color.ambient);
    shader.setVec3("DirLight.diffuse"  , color.diffuse);
    shader.setVec3("DirLight.specular" , color.specular);

    shader.setVec3("DirLight.direction", direction);
}

void PointLight::sendToShader(Shader& shader)
{
    shader.setVec3("PointLight.ambient"  , color.ambient);
    shader.setVec3("PointLight.diffuse"  , color.diffuse);
    shader.setVec3("PointLight.specular" , color.specular);

    shader.setVec3("PointLight.direction", position);

    shader.setFloat("PointLight.constant" , attenuation.constant);
    shader.setFloat("PointLight.linear"   , attenuation.linear);
    shader.setFloat("PointLight.quadratic", attenuation.quadratic);
}

void Spotlight::sendToShader(Shader& shader)
{
    shader.setVec3("PointLight.ambient"  , color.ambient);
    shader.setVec3("PointLight.diffuse"  , color.diffuse);
    shader.setVec3("PointLight.specular" , color.specular);

    shader.setVec3("DirLight.direction", direction);
    shader.setVec3("PointLight.direction", position);

    shader.setFloat("PointLight.constant" , attenuation.constant);
    shader.setFloat("PointLight.linear"   , attenuation.linear);
    shader.setFloat("PointLight.quadratic", attenuation.quadratic);
}

constexpr LightColor white {
    glm::vec3(0.2f),    // Ambient
    glm::vec3(0.8f),    // Diffuse
    glm::vec3(1.0f)     // Specular
};

constexpr LightAttenuation standardAttenuation {
    1.0f,               // Constant
    0.09f,              // Linear
    0.032f              // Quadratic
};

constexpr glm::vec3 standardDirection(-0.2f, -1.0f, -0.3f);

DirectionaLight LightingEngine::DefaultDirectionalLight() {
    return DirectionaLight(white, standardDirection);
}

PointLight LightingEngine::DefaultPointLight(glm::vec3 position) {
    return PointLight(white, standardAttenuation, position);
}

Spotlight LightingEngine::DefaultSpotlight(Camera& cam) {
    return Spotlight(white, standardAttenuation, cam.position, cam.front);
}