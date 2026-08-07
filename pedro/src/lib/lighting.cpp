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
    std::string strDir = "PointLight[" + std::to_string(index) + "].";

    shader.setVec3((strDir + "ambient").c_str()   , color.ambient);
    shader.setVec3((strDir + "diffuse").c_str()   , color.diffuse);
    shader.setVec3((strDir + "specular").c_str()  , color.specular);

    shader.setVec3((strDir + "direction").c_str() , position);

    shader.setFloat((strDir + "constant").c_str() , attenuation.constant);
    shader.setFloat((strDir + "linear").c_str()   , attenuation.linear);
    shader.setFloat((strDir + "quadratic").c_str(), attenuation.quadratic);
}

void Spotlight::sendToShader(Shader& shader)
{
    shader.setVec3("Spotlight.ambient"   , color.ambient);
    shader.setVec3("Spotlight.diffuse"   , color.diffuse);
    shader.setVec3("Spotlight.specular"  , color.specular);
    shader.setVec3("Spotlight.direction" , direction);
    shader.setVec3("Spotlight.direction" , position);

    shader.setFloat("Spotlight.constant" , attenuation.constant);
    shader.setFloat("Spotlight.linear"   , attenuation.linear);
    shader.setFloat("Spotlight.quadratic", attenuation.quadratic);
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