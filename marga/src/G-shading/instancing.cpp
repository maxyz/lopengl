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
        .title = "Many Instances",
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
        Shader  *instanceShader;
        // Helper shader structures and data
        void createShaders();

        unsigned int quadVAO, quadVBO;
        unsigned int rows, cols;
        unsigned int lastIndex;
        glm::vec2 translations[2500];
        void calculateOffsets();
        bool useInstanceShader = false;
        void drawManyQuads();
        void useInstancing();

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
    this->sceneShader = new Shader("shaders/vertex.glsl", "shaders/instances-frag.glsl");
    this->instanceShader = new Shader("shaders/instances-vertex.glsl", "shaders/instances-frag.glsl");
}

void SceneRenderer::init()
{
    this->setOptions();
    this->createShaders();
    this->cols = 30;
    this->rows = 30;

    // This is here because it's only used for this code
    float quadVertices[] = {
        // positions     // colors
        -0.02f,  0.02f,  1.0f, 0.0f, 0.0f,
         0.02f, -0.02f,  0.0f, 1.0f, 0.0f,
        -0.02f, -0.02f,  0.0f, 0.0f, 1.0f,
    
        -0.02f,  0.02f,  1.0f, 0.0f, 0.0f,
         0.02f, -0.02f,  0.0f, 1.0f, 0.0f,   
         0.02f,  0.02f,  0.0f, 1.0f, 1.0f		    		
    };
    createVertexBuffers(&this->quadVAO, &this->quadVBO, &quadVertices, sizeof(quadVertices));
    setVertexAttribs(2,3,0);
    
}

void SceneRenderer::calculateOffsets() {
    if (this->lastIndex == this->rows * this->cols) {
        return;
    }
    unsigned int index = 0;
    float maxrows = (float) this->rows;
    float maxcols = (float) this->cols;
    for(float y = 0.5; y < maxrows; y += 1.0)
    {
        for(float x = 0.5; x < maxcols; x += 1.0)
        {
            glm::vec2 translation;
            translation.x = -1.0 + (x / maxcols)*2;
            translation.y = -1.0 + (y / maxrows)*2;
            this->translations[index++] = translation;
        }
    }
    this->lastIndex = index;

    // Set the values in the shader
    this->instanceShader->use();
    for(unsigned int i = 0; i < index; i++)
    {
        this->instanceShader->setVec2f(("offsets[" + std::to_string(i) + "]"), glm::value_ptr(translations[i]));
    } 
}

// Old school way: draw many quads
void SceneRenderer::drawManyQuads() {
    this->sceneShader->use();
    glm::mat4 model;
    // For each index, translate the model and render the quad
    for(unsigned int i = 0; i < this->lastIndex; i++) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(this->translations[i], 0.0));
        this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

// Instancing way: draw one quad and let the shaders multiply.
void SceneRenderer::useInstancing() {
    this->instanceShader->use();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, this->lastIndex); 
}


void SceneRenderer::renderScene(SceneState &state) 
{
    glClearColor(state.bgColor.x, state.bgColor.y, state.bgColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(this->quadVAO);
    this->calculateOffsets();

    if (this->useInstanceShader) {
        this->useInstancing();
    } else {
        this->drawManyQuads();
    }

}

void SceneRenderer::showImGuiControls(SceneState &state) {
    unsigned int min = 2;
    unsigned int max = 50;
    ImGui::SliderScalar("Rows", ImGuiDataType_U32, &this->rows, &min, &max, "%u");
    ImGui::SliderScalar("Cols", ImGuiDataType_U32, &this->cols, &min, &max, "%u");
    ImGui::Checkbox("Use instancing", &this->useInstanceShader);
}

void SceneRenderer::teardown()
{
    // Clean up buffers
    glDeleteVertexArrays(1, &this->quadVAO);
    glDeleteBuffers(1, &this->quadVBO);
}


