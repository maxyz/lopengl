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
#include "model.h"
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
        .title = "Anti Aliasing: Green Cube",
        .bgColor = glm::vec3( 0.1f,  0.1f,  0.4f),
        .camera = Camera(glm::vec3(2.0f, 0.0f, 5.0f)),
        .lastX = 400,
        .lastY = 300,
        .firstMouse = true,
        .shininess = 32.0,
};

class SceneRenderer: public AbstractSceneRenderer {
    private:
        Shader  *sceneShader;
        void createShaders();
        void createBuffers();
        void setOptions();
        unsigned int cubeVAO, cubeVBO;
        glm::vec3 cubeColor = glm::vec3( 0.0f,  0.8f,  0.0f);

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
    glEnable(GL_MULTISAMPLE);
}

void SceneRenderer::createShaders()
{
    this->sceneShader = new Shader("shaders/vertex.glsl", "shaders/frag.glsl");
}

void SceneRenderer::init()
{
    this->setOptions();
    this->createShaders();
    this->createBuffers();
}

void SceneRenderer::createBuffers()
{
    getCubeBuffers(&this->cubeVAO, &this->cubeVBO);
}

void SceneRenderer::renderScene(SceneState &state) 
{
    glClearColor(state.bgColor.x, state.bgColor.y, state.bgColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = state.camera.GetViewMatrix(); // Full view
    glm::mat4 projection = glm::perspective(glm::radians(state.camera.Zoom), state.width/state.height, 0.1f, 1000.0f);
    glm::mat4 model = glm::mat4(1.0f);
    this->sceneShader->use();
    this->sceneShader->setMatrix4fv("view", glm::value_ptr(view));
    this->sceneShader->setMatrix4fv("projection", glm::value_ptr(projection));
    this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
    this->sceneShader->setVec3f("color", glm::value_ptr(cubeColor));

    glBindVertexArray(this->cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

}

void SceneRenderer::showImGuiControls(SceneState &state) {
    ImGui::ColorEdit3("Cube Color", glm::value_ptr(this->cubeColor));
}

void SceneRenderer::teardown()
{
}


