#include "lights.h"

Light::Light(glm::vec3 color, float ambient, float diffuse, float specular):
        color(color), ambientStrength(ambient), diffuseStrength(diffuse), specularStrength(specular) {}

Light::Light(float ambient, float diffuse):
        Light(glm::vec3( 1.0f,  1.0f,  1.0f), ambient, diffuse, 1.0) {}

void Light::setBasicValues(Shader lightShader, std::string name) {
    glm::vec3 diffuseColor = this->color  * this->diffuseStrength;
    glm::vec3 ambientColor = diffuseColor * this->ambientStrength;
    glm::vec3 specularColor = this->color * this->specularStrength;

    lightShader.setVec3f(std::format("{}.ambient", name), glm::value_ptr(ambientColor));
    lightShader.setVec3f(std::format("{}.diffuse", name), glm::value_ptr(diffuseColor));
    lightShader.setVec3f(std::format("{}.specular", name), glm::value_ptr(specularColor));
}

/****** Directional ******/

DirectionalLight::DirectionalLight(glm::vec3 direction, glm::vec3 color, float ambient, float diffuse, float specular):
        direction(direction), Light(color, ambient, diffuse, specular) { }

DirectionalLight::DirectionalLight(glm::vec3 direction, float ambient, float diffuse):
        direction(direction), Light(ambient, diffuse) { }
 
DirectionalLight::DirectionalLight(glm::vec3 direction):
        DirectionalLight(direction, 0.2f, 0.5f) { }

void DirectionalLight::setShaderValues(Shader lightShader, std::string name) {
    setBasicValues(lightShader, name);
    lightShader.setVec3f(std::format("{}.direction", name), glm::value_ptr(this->direction));
}

void DirectionalLight::showImGuiControls(std::string header) {
    if (ImGui::CollapsingHeader(header.c_str())) {
        ImGui::DragFloat3("Dir Light direction", glm::value_ptr(this->direction), 0.01f, -10.0f, 10.0f);
        ImGui::ColorEdit3("Dir Light Color", glm::value_ptr(this->color));
        ImGui::SliderFloat("Dir Light Ambience", &this->ambientStrength, -1.0f, 1.0f);
        ImGui::SliderFloat("Dir Light Diffuse", &this->diffuseStrength, -1.0f, 1.0f);
        ImGui::SliderFloat("Dir Light Specular", &this->specularStrength, -1.0f, 1.0f);
    }
}


/****** Positional ******/

PositionalLight::PositionalLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse, float specular, float constant, float linear, float quadratic):
        position(position), Light(color, ambient, diffuse, specular), constant(constant), linear(linear), quadratic(quadratic) { }

PositionalLight::PositionalLight(glm::vec3 position, glm::vec3 color, float ambient, float diffuse):
        position(position), Light(color, ambient, diffuse, 1.0), constant(1.0), linear(0.09f), quadratic(0.032f) { }

PositionalLight::PositionalLight(glm::vec3 position):
        PositionalLight(position, glm::vec3( 1.0f,  1.0f,  1.0f), 0.1f, 0.4f) {}

void PositionalLight::setShaderValues(Shader lightShader, std::string name) {
    setBasicValues(lightShader, name);

    lightShader.setFloat(std::format("{}.constant", name), this->constant);
    lightShader.setFloat(std::format("{}.linear", name), this->linear);
    lightShader.setFloat(std::format("{}.quadratic", name), this->quadratic);
    lightShader.setVec3f(std::format("{}.position", name), glm::value_ptr(this->position));
}

void PositionalLight::showImGuiControls(std::string header) {
    if (ImGui::CollapsingHeader(header.c_str())) {
            ImGui::ColorEdit3("Light Color", glm::value_ptr(this->color));
            ImGui::SliderFloat("Light Ambience", &this->ambientStrength, -1.0f, 1.0f);
            ImGui::SliderFloat("Light Diffuse", &this->diffuseStrength, -1.0f, 1.0f);
            ImGui::SliderFloat("Light Specular", &this->specularStrength, -1.0f, 1.0f);
            ImGui::DragFloat3("Light Position", glm::value_ptr(this->position));
            ImGui::SliderFloat("Attenuation Constant", &this->constant, 0.0f, 1.0f);
            ImGui::SliderFloat("Attenuation Linear", &this->linear, 0.0f, 1.0f);
            ImGui::SliderFloat("Attenuation Quadratic", &this->quadratic, 0.0f, 1.0f);
    }
}

/****** Spotlight ******/

SpotLight::SpotLight(glm::vec3 color, float ambient, float diffuse, float specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff):
        PositionalLight(glm::vec3( 0.0f,  0.0f,  3.0f), color, ambient, diffuse, specular, constant, linear, quadratic),
        cutOff(cutOff), outerCutOff(outerCutOff) { }

SpotLight::SpotLight(float ambient, float diffuse, float cutOff, float outerCutOff):
        PositionalLight(glm::vec3( 0.0f,  0.0f,  3.0f), glm::vec3( 1.0f,  1.0f,  1.0f), 
                        ambient, diffuse, 1.0, 1.0, 0.09f, 0.032f),
        cutOff(cutOff), outerCutOff(outerCutOff) { }

SpotLight::SpotLight(float cutOff, float outerCutOff):
        SpotLight(0.5f, 0.9f, cutOff, outerCutOff) { }

SpotLight::SpotLight():
        SpotLight(12.5f, 15.0f) { }

void SpotLight::setShaderValues(Shader lightShader, std::string name) {
    PositionalLight::setShaderValues(lightShader, name);
    lightShader.setVec3f(std::format("{}.direction", name), glm::value_ptr(this->direction));
    lightShader.setFloat(std::format("{}.cutOff", name),
                    glm::cos(glm::radians(this->cutOff)));
    lightShader.setFloat(std::format("{}.outerCutOff", name),
                    glm::cos(glm::radians(this->outerCutOff)));
}

void SpotLight::showImGuiControls(std::string header) {
    if (ImGui::CollapsingHeader(header.c_str())) {
            ImGui::ColorEdit3("Spot Light Color", glm::value_ptr(this->color));
            ImGui::SliderFloat("Spot Light Ambience", &this->ambientStrength, -1.0f, 1.0f);
            ImGui::SliderFloat("Spot Light Diffuse", &this->diffuseStrength, -1.0f, 1.0f);
            ImGui::SliderFloat("Spot Light Specular", &this->specularStrength, -1.0f, 1.0f);
            ImGui::SliderFloat("Spot Attenuation Constant", &this->constant, 0.0f, 1.0f);
            ImGui::SliderFloat("Spot Attenuation Linear", &this->linear, 0.0f, 1.0f);
            ImGui::SliderFloat("Spot Attenuation Quadratic", &this->quadratic, 0.0f, 1.0f);
            ImGui::InputFloat("Spot Cut Off angle (in degrees)", &this->cutOff, 1.0f, 10.0f);
            ImGui::InputFloat("Spot Outer Cut Off angle (in degrees)", &this->outerCutOff, 1.0f, 10.0f);
    }
}

/* Presets */
void SetLights(std::string name, glm::vec3 &backgroundColor, DirectionalLight &directionalLight, PositionalLight positionalLights[], SpotLight &spotLight) {

    glm::vec3 positionalColor;
    float positionalAmbient;
    float positionalDiffuse;
    float positionalAttLin;
    float positionalAttQua;


    if (name == "Desert") {
        backgroundColor = glm::vec3(226.0f/256.0f, 172.0f/256.0f, 91.0f/256.0f);
        directionalLight.color = glm::vec3( 254.0f/256.0f, 180.0f/256.0f, 141.0f/256.0f);
        directionalLight.ambientStrength = 0.5f;
        directionalLight.diffuseStrength = 0.9f;
        directionalLight.specularStrength = 1.0f;

        positionalColor = glm::vec3( 0.98f,  0.88f,  0.55f);
        positionalAmbient = 0.5f;
        positionalDiffuse = 0.9f;
        positionalAttLin = 0.09;
        positionalAttQua = 0.032;

        spotLight.diffuseStrength = 0.7f;
        spotLight.ambientStrength = 0.2f;
        spotLight.cutOff = 12.5f;
        spotLight.outerCutOff = 15.0f;
    } else if (name == "Factory") {
        backgroundColor = glm::vec3(0.08f, 0.09f, 0.15f);
        directionalLight.color = glm::vec3( 0.7f,  0.71f,  0.98f);
        directionalLight.ambientStrength = 0.15f;
        directionalLight.diffuseStrength = 0.45f;
        directionalLight.specularStrength = 0.5f;

        positionalColor = glm::vec3( 0.0f,  0.0f,  0.55f);
        positionalAmbient = 0.15f;
        positionalDiffuse = 0.5f;
        positionalAttLin = 0.09;
        positionalAttQua = 0.032;

        spotLight.diffuseStrength = 1.0f;
        spotLight.ambientStrength = 0.2f;
        spotLight.cutOff = 12.5f;
        spotLight.outerCutOff = 15.0f;

    } else if (name == "Horror") {
        backgroundColor = glm::vec3(0.09f, 0.08f, 0.11f);
        directionalLight.color = glm::vec3( 38.0f/256.0f, 30.0f/256.0f, 96.0f/256.0f);
        directionalLight.ambientStrength = 0.01f;
        directionalLight.diffuseStrength = 0.1f;
        directionalLight.specularStrength = 0.1f;

        positionalColor = glm::vec3( 0.35f,  0.22f,  0.40f);
        positionalAmbient = 0.15f;
        positionalDiffuse = 0.5f;
        positionalAttLin = 0.1;
        positionalAttQua = 0.04;

        spotLight.diffuseStrength = 0.5f;
        spotLight.ambientStrength = 0.0f;
        spotLight.cutOff = 5.0f;
        spotLight.outerCutOff = 8.0f;
    } else if (name == "BioLab") {
        backgroundColor = glm::vec3(1.0f, 1.0f, 1.0f);
        directionalLight.color = glm::vec3( 0.7f,  0.71f,  0.98f);
        directionalLight.ambientStrength = 0.3f;
        directionalLight.diffuseStrength = 0.6f;
        directionalLight.specularStrength = 1.0f;

        positionalColor = glm::vec3( 0.63f,  0.98f,  0.63f);;
        positionalAmbient = 0.5f;
        positionalDiffuse = 0.9f;
        positionalAttLin = 0.03;
        positionalAttQua = 0.03;

        spotLight.diffuseStrength = 0.7f;
        spotLight.ambientStrength = 0.2f;
        spotLight.cutOff = 25.0f;
        spotLight.outerCutOff = 30.0f;
    }

    for (int i = 0; i < 4; i++) {
        positionalLights[i].color.r = positionalColor.r;
        positionalLights[i].color.g = positionalColor.g;
        positionalLights[i].color.b = positionalColor.b;
        positionalLights[i].ambientStrength = positionalAmbient;
        positionalLights[i].diffuseStrength = positionalDiffuse;
        positionalLights[i].linear = positionalAttLin;
        positionalLights[i].quadratic = positionalAttQua;
    }


}


