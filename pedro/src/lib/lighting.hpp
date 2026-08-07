#ifndef LIGHTING_HPP
#define LIGHTING_HPP

#include "shader.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <memory>

struct LightColor
{
    glm::vec3 ambient, diffuse, specular;
};

struct LightAttenuation
{
    float constant, linear, quadratic;
};

class Light
{
public:
    ~Light() = default;
    virtual void sendToShader(Shader& shader) = 0;
    // virtual void renderLightSource(Shader& lightsourceShader) = 0;
};

class DirectionaLight : public Light 
{
public:
    DirectionaLight(LightColor color, glm::vec3 direction) : 
    color(color),
    direction(direction)
    {}

    LightColor color;
    glm::vec3 direction;

    void sendToShader(Shader& shader) override;
};

class PointLight : public Light
{
public:
    PointLight(LightColor color, LightAttenuation attenuation, glm::vec3 position, size_t index = 0) : 
    color(color),
    attenuation(attenuation),
    position(position),
    index(index)
    {}

    size_t index;
    LightColor color;
    LightAttenuation attenuation;
    glm::vec3 position;

    void sendToShader(Shader& shader) override;
};

class Spotlight : public Light
{
public:
    Spotlight(LightColor color, LightAttenuation attenuation, glm::vec3 position, glm::vec3 direction) : 
    color(color),
    attenuation(attenuation),
    position(position),
    direction(direction)
    {}

    LightColor color;
    LightAttenuation attenuation;
    glm::vec3 direction;
    glm::vec3 position;
    
    void sendToShader(Shader& shader) override;
};

using LightPtr = std::unique_ptr<Light>;
using LightMap = std::unordered_map<std::string, LightPtr>;

// todo: figure out wtf do i do with the index things. maybe check out the uniforms??
class LightingEngine
{
private:
    uint pointLightAmount = 0;
    
public:
    LightingEngine() = default;
    ~LightingEngine() = default;

    LightMap lights;
    
    void sendToShader(Shader& shader)
    {
        for(auto& [_, light] : lights) light->sendToShader(shader);
    }

    void addPointLight(std::string name, PointLight pointLight) {
        pointLight.index = pointLightAmount;
        lights[name] = std::make_unique<PointLight>(pointLight);
        pointLightAmount++;
    }

    void addSpotlight(std::string name, Spotlight spotlight)
    {
        lights[name] = std::make_unique<Spotlight>(spotlight);
    }

    void addDirectionalLight(std::string name, DirectionaLight dirLight)
    {
        lights[name] = std::make_unique<DirectionaLight>(dirLight);
    }

    static DirectionaLight DefaultDirectionalLight();
    static PointLight DefaultPointLight(glm::vec3 position);
    static Spotlight DefaultSpotlight(Camera& cam);
};

#endif