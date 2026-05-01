#ifndef __LIGHTS_H
#define __LIGHTS_H

#include <string>
#include <format>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

#include "shader.h"

class Light {
public:
    glm::vec3 color;
    float ambientStrength;
    float diffuseStrength;
    float specularStrength;

    Light(glm::vec3 color, float ambient, float diffuse, float specular);
    Light(float ambient, float diffuse);
protected:
    void setBasicValues(Shader lightShader, std::string name);
};

class DirectionalLight: public Light {
public:
    glm::vec3 direction;

    DirectionalLight(glm::vec3 direction);
    DirectionalLight(glm::vec3 direction, float ambient, float diffuse);
    DirectionalLight(glm::vec3 direction, glm::vec3 color, float ambient, float diffuse, float specular);

    void setShaderValues(Shader lightShader, std::string name);
    void showImGuiControls(std::string header);

};

class PositionalLight: public Light {
public:
    glm::vec3 position;

    float constant;
    float linear;
    float quadratic;

    PositionalLight(glm::vec3 position);
    PositionalLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse);
    PositionalLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular, float constant, float linear, float quadratic);

    void setShaderValues(Shader lightShader, std::string name);
    void showImGuiControls(std::string header);
};

class SpotLight: public PositionalLight {
public:
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    SpotLight();
    SpotLight(float cutOff, float outerCutOff);
    SpotLight(float ambient, float diffuse, float cutOff, float outerCutOff);
    SpotLight(glm::vec3 color, float ambient, float diffuse, float specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff);

    void setShaderValues(Shader lightShader, std::string name);
    void showImGuiControls(std::string header);
};

void SetLights(std::string name, glm::vec3 &backgroundColor, DirectionalLight &directionalLight, PositionalLight positionalLights[], SpotLight &spotLight);





#endif
