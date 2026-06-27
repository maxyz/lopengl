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
        .title = "Frag Coord",
        .bgColor = glm::vec3( 0.1f,  0.1f,  0.1f),
        .camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f)),
        .lastX = 400,
        .lastY = 300,
        .firstMouse = true,
        .shininess = 32.0,
};

class SceneRenderer: public AbstractSceneRenderer {
    private:
        // Main Shaders
        Shader  *sceneShader;
        // Helper shader structures and data
        void createShaders();

        unsigned int cubeVAO, cubeVBO;
        // Colors of the cubes
        glm::vec3 leftColor;
        glm::vec3 rightColor;

        void setOptions();

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
    // Point size in the shader
    //glEnable(GL_PROGRAM_POINT_SIZE);
    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void SceneRenderer::createShaders()
{
    // Set the main attributes
    this->sceneShader = new Shader("shaders/fragcoord-vertex.glsl", "shaders/fragcoord-frag.glsl");
}

void SceneRenderer::init()
{
    this->setOptions();
    this->createShaders();
    // VAOs and VBOs
    getCubeBuffers(&this->cubeVAO, &this->cubeVBO);
    this->leftColor = glm::vec3( 0.9f,  0.1f,  0.1f);
    this->rightColor = glm::vec3( 0.1f,  0.1f,  0.9f);
}

void SceneRenderer::renderScene(SceneState &state) 
{
    glClearColor(state.bgColor.x, state.bgColor.y, state.bgColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Coordinate matrixes
    // ** View **
    glm::mat4 view = state.camera.GetViewMatrix(); // Full view
    // ** Projection **
    glm::mat4 projection = glm::perspective(glm::radians(state.camera.Zoom), state.width/state.height, 0.1f, 100.0f);

    // Now the rest of the scene 
    this->sceneShader->use();
    this->sceneShader->setVec3f("viewPos", glm::value_ptr(state.camera.Position));
    this->sceneShader->setVec3f("leftColor", glm::value_ptr(this->leftColor));
    this->sceneShader->setVec3f("rightColor", glm::value_ptr(this->rightColor));
    this->sceneShader->setFloat("halfScreen", state.width/2);

    this->sceneShader->setMatrix4fv("view", glm::value_ptr(view));
    this->sceneShader->setMatrix4fv("projection", glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);

    // Draw 2 cubes
    glBindVertexArray(this->cubeVAO);
    model = glm::translate(model, glm::vec3(-1.5f, 0.0f, -1.5f));
    this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.5f, 0.0f, -0.5f));
    this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

}

void SceneRenderer::showImGuiControls(SceneState &state) {
    // Point color selector
    ImGui::ColorEdit3("Left Color", glm::value_ptr(this->leftColor));
    ImGui::ColorEdit3("Right Color", glm::value_ptr(this->rightColor));
}

void SceneRenderer::teardown()
{
    // Clean up buffers
    glDeleteVertexArrays(1, &this->cubeVAO);
    glDeleteBuffers(1, &this->cubeVBO);
}


