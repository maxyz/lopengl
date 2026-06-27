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
        .title = "Uniform buffer objects",
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
        Shader  *redShader;
        Shader  *greenShader;
        Shader  *blueShader;
        Shader  *yellowShader;
        // Helper shader structures and data
        void createShaders();

        void setOptions();

        unsigned int cubeVAO, cubeVBO;

        // UBO
        unsigned int uboMatrices;
        void createUBOs();

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
    // Four plain shaders, to simulate more complex ones
    this->redShader = new Shader("shaders/ubo-vertex.glsl", "shaders/ubo-red-frag.glsl");
    this->greenShader = new Shader("shaders/ubo-vertex.glsl", "shaders/ubo-green-frag.glsl");
    this->blueShader = new Shader("shaders/ubo-vertex.glsl", "shaders/ubo-blue-frag.glsl");
    this->yellowShader = new Shader("shaders/ubo-vertex.glsl", "shaders/ubo-yellow-frag.glsl");
}

void SceneRenderer::init()
{
    this->setOptions();
    this->createShaders();
    this->createUBOs();
    // VAOs and VBOs
    getCubeBuffers(&this->cubeVAO, &this->cubeVBO);
}

// Initialize the uniform buffer objects
void SceneRenderer::createUBOs()
{
    // Set the uniform block of the vertex shaders equal to binding point 0.
    // Note that we have to do this for each shader
    unsigned int uniformBlockIndexRed    = glGetUniformBlockIndex(this->redShader->ID, "Matrices");
    unsigned int uniformBlockIndexGreen  = glGetUniformBlockIndex(this->greenShader->ID, "Matrices");
    unsigned int uniformBlockIndexBlue   = glGetUniformBlockIndex(this->blueShader->ID, "Matrices");
    unsigned int uniformBlockIndexYellow = glGetUniformBlockIndex(this->yellowShader->ID, "Matrices");

    glUniformBlockBinding(this->redShader->ID,    uniformBlockIndexRed, 0);
    glUniformBlockBinding(this->greenShader->ID,  uniformBlockIndexGreen, 0);
    glUniformBlockBinding(this->blueShader->ID,   uniformBlockIndexBlue, 0);
    glUniformBlockBinding(this->yellowShader->ID, uniformBlockIndexYellow, 0);

    // create the actual uniform buffer object and bind that buffer to binding point 0
    glGenBuffers(1, &this->uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, this->uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, this->uboMatrices, 0, 2 * sizeof(glm::mat4));    
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

    // Store the matrixes in the UBO
    glBindBuffer(GL_UNIFORM_BUFFER, this->uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    glm::mat4 model = glm::mat4(1.0f);

    // Draw 4 cubes, one with each shader
    glBindVertexArray(this->cubeVAO);
    model = glm::translate(model, glm::vec3(-1.5f, 0.0f, -1.5f));
    this->redShader->use();
    this->redShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.5f, 0.0f, -1.5f));
    this->greenShader->use();
    this->greenShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.5f, 0.0f, 2.5f));
    this->blueShader->use();
    this->blueShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 0.0f, 2.5f));
    this->yellowShader->use();
    this->yellowShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

}

void SceneRenderer::showImGuiControls(SceneState &state) {
}

void SceneRenderer::teardown()
{
    // Clean up buffers
    glDeleteVertexArrays(1, &this->cubeVAO);
    glDeleteBuffers(1, &this->cubeVBO);
}


