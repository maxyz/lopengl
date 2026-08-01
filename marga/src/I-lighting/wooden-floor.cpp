#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <map>
#include <format>
#include <cmath>
#include <unordered_map>
#include "stb_image.h"
#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "lights.h"
#include "geometry.h"
#include "scene_state.h"
#include "imgui_dock.h"
#include "basic_main.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>

const int INITIAL_WIDTH = 1024;
const int INITIAL_HEIGHT = 768;

SceneState state = {
        .width = (float) INITIAL_WIDTH,
        .height = (float) INITIAL_HEIGHT,
        .title = "Lighting experiment: Wooden floor reflection",
        .bgColor = glm::vec3( 0.1f,  0.1f,  0.1f),
        .camera = Camera(glm::vec3(2.0f, 3.0f, 10.0f)),
        .lastX = 400,
        .lastY = 300,
        .firstMouse = true,
        .shininess = 8.0,
};

class SceneRenderer: public AbstractSceneRenderer {
    private:
        Shader  *sceneShader;
        Shader  *sourceShader;
        void createShaders();
        void createBuffers();
        void setOptions();

        // Wooden floor
        Texture *floorMaterial;
        unsigned int planeVAO, planeVBO;

        LightSet *lights;
        unsigned int lightVAO, lightVBO;
        void createLights();

        // Used to enable/disable Blinn-Phong
        bool blinn = false;

    public:
        SceneRenderer() {}
        void init();
        void renderScene(SceneState &state);
        void showImGuiControls(SceneState &state);
        void teardown();
};

AbstractSceneRenderer* createSceneRenderer() {
    return new SceneRenderer();
}


// OpenGL options that we want to use in this program
void SceneRenderer::setOptions()
{
    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);
    //glEnable(GL_MULTISAMPLE);
}

void SceneRenderer::createShaders()
{
    this->sourceShader = new Shader("shaders/source-vertex.glsl", "shaders/source-frag.glsl");
    this->sceneShader = new Shader("shaders/vertex.glsl", "shaders/blinn.glsl");
}

void SceneRenderer::init()
{
    this->setOptions();
    this->createShaders();
    this->createBuffers();
    this->createLights();

    Texture::flip_vertically();
    this->floorMaterial = new Texture("../media/wood.png");

    this->sceneShader->use();
    this->sceneShader->setInt("material.texture_diffuse1", 0);
    this->sceneShader->setInt("material.texture_specular1", 1);
}

void SceneRenderer::createBuffers()
{
    getPlaneBuffers(&this->planeVAO, &this->planeVBO);
    getCubeBuffers(&this->lightVAO, &this->lightVBO);
}

void SceneRenderer::createLights()
{
    // Starting lighting values (position, color, ambient, diffuse, specular, constant, linear, quadratic, cutoff)
    DirectionalLight directionalLight(glm::vec3(-0.2f, 1.0f, 0.3f));
    SpotLight spotLight;
    directionalLight.active = false;
    spotLight.active = false;
    std::array<PositionalLight, 4> positionalLights = {{
        PositionalLight(glm::vec3( 1.7f,  1.2f, -2.0f), glm::vec3( 1.0f,  1.0f,  1.0f), 0.2f, 0.7f),
        PositionalLight(glm::vec3( 4.3f,  0.5f, -4.0f), glm::vec3( 1.0f,  1.0f,  1.0f), 0.2f, 0.7f),
        PositionalLight(glm::vec3(-4.0f,  2.0f, -2.0f), glm::vec3( 1.0f,  1.0f,  1.0f), 0.2f, 0.7f),
        PositionalLight(glm::vec3(-3.0f,  0.0f,  3.0f), glm::vec3( 1.0f,  1.0f,  1.0f), 0.2f, 0.7f)
    }};
    this->lights = new LightSet(directionalLight, spotLight, 4, positionalLights);
}

void SceneRenderer::renderScene(SceneState &state) 
{
    glClearColor(state.bgColor.x, state.bgColor.y, state.bgColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->sceneShader->use();
    this->sceneShader->setVec3f("viewPos", glm::value_ptr(state.camera.Position));
    this->sceneShader->setFloat("material.shininess", state.shininess);
    this->sceneShader->setBool("blinn", this->blinn);

    // Store the light values in the scene shader
    lights = this->lights;
    lights->directionalLight.setShaderValues(*this->sceneShader, "dirLight");

    for (int i = 0; i < lights->positionalLightAmount; i++) {
        lights->positionalLights[i].setShaderValues(*this->sceneShader, std::format("pointLights[{}]", i));
    }

    lights->spotLight.position = state.camera.Position;
    lights->spotLight.direction = state.camera.Front;
    lights->spotLight.setShaderValues(*this->sceneShader, "spotLight");

    glm::mat4 view = state.camera.GetViewMatrix(); // Full view
    glm::mat4 projection = glm::perspective(glm::radians(state.camera.Zoom), state.width/state.height, 0.1f, 100.0f);
    glm::mat4 model;

    this->sceneShader->setMatrix4fv("view", glm::value_ptr(view));
    this->sceneShader->setMatrix4fv("projection", glm::value_ptr(projection));

    // Draw the lights on the screen
    this->sourceShader->use();
    this->sourceShader->setMatrix4fv("view", glm::value_ptr(view));
    this->sourceShader->setMatrix4fv("projection", glm::value_ptr(projection));

    for (int i = 0; i < lights->positionalLightAmount; i++) {
        PositionalLight light = lights->positionalLights[i];
        if (not light.active) continue;
        model = glm::mat4(1.0f);
        model = glm::translate(model, light.position);
        model = glm::scale(model, glm::vec3(0.2f));
        this->sourceShader->setMatrix4fv("model", glm::value_ptr(model));
        this->sourceShader->setVec3f("lightColor", glm::value_ptr(light.color));
        // draw the light cube object
        glBindVertexArray(this->lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // floor
    this->sceneShader->use();
    glBindVertexArray(this->planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->floorMaterial->ID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->floorMaterial->ID);
    model = glm::mat4(1.0f);
    this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 6);

}

void SceneRenderer::showImGuiControls(SceneState &state) {
    this->lights->showImGuiControls(state);
    ImGui::Checkbox("Enable Blinn-Phong", &(this->blinn));
}

void SceneRenderer::teardown()
{
    glDeleteVertexArrays(1, &this->planeVAO);
    glDeleteVertexArrays(1, &this->lightVAO);
    glDeleteBuffers(1, &this->planeVBO);
    glDeleteBuffers(1, &this->lightVBO);
}


