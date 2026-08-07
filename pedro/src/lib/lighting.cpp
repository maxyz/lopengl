#include "lighting.hpp"

void DirectionaLight::sendToShader(Shader& shader)
{
    shader.setVec3("dirLight.ambient"  , color.ambient);
    shader.setVec3("dirLight.diffuse"  , color.diffuse);
    shader.setVec3("dirLight.specular" , color.specular);

    shader.setVec3("dirLight.direction", direction);
}

void PointLight::sendToShader(Shader& shader)
{
    std::string strDir = "pointLights[" + std::to_string(index) + "].";

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
    shader.setVec3("spotlight.ambient"   , color.ambient);
    shader.setVec3("spotlight.diffuse"   , color.diffuse);
    shader.setVec3("spotlight.specular"  , color.specular);
    shader.setVec3("spotlight.direction" , direction);
    shader.setVec3("spotlight.direction" , position);

    shader.setFloat("spotlight.constant" , attenuation.constant);
    shader.setFloat("spotlight.linear"   , attenuation.linear);
    shader.setFloat("spotlight.quadratic", attenuation.quadratic);
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