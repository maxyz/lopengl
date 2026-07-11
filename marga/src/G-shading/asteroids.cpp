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
        .title = "Asteroids",
        .bgColor = glm::vec3( 0.1f,  0.1f,  0.1f),
        .camera = Camera(glm::vec3(0.0f, 0.0f, 75.0f)),
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

        void setOptions();

        Model *planet;
        Model *asteroid;
        glm::mat4 *modelMatrices;
        unsigned int amount = 1000;
        void createAsteroids();
        unsigned int lastIndex = 0;

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
    glEnable(GL_DEPTH_TEST);
}

void SceneRenderer::createShaders()
{
    this->sceneShader = new Shader("shaders/simple-model-vertex.glsl", "shaders/model-frag.glsl");
}

void SceneRenderer::init()
{
    this->setOptions();
    this->createShaders();
    this->planet = new Model("../media/models/planet.obj");
    this->asteroid = new Model("../media/models/rock.obj");
    state.camera.MovementSpeed = 40.0;
}

void SceneRenderer::createAsteroids()
{
    if (this->lastIndex == this->amount) return;
    if (this->lastIndex > 0) {
        delete this->modelMatrices;
    }
    this->modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime()); // initialize random seed	
    float radius = 50.0;
    float offset = 2.5f;
    for(unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));
    
        // 2. scale: scale between 0.05 and 0.25f
        float scale = (rand() % 20) / 100.0f + 0.05;
        model = glm::scale(model, glm::vec3(scale));
    
        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
    
        // 4. now add to list of matrices
        this->modelMatrices[i] = model;
    }
    this->lastIndex = this->amount;
}

void SceneRenderer::renderScene(SceneState &state) 
{
    glClearColor(state.bgColor.x, state.bgColor.y, state.bgColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->sceneShader->use();
    glm::mat4 view = state.camera.GetViewMatrix(); // Full view
    glm::mat4 projection = glm::perspective(glm::radians(state.camera.Zoom), state.width/state.height, 0.1f, 1000.0f);
    glm::mat4 model = glm::mat4(1.0f);

    this->sceneShader->setMatrix4fv("view", glm::value_ptr(view));
    this->sceneShader->setMatrix4fv("projection", glm::value_ptr(projection));

    // draw the planet
    model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
    this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
    this->planet->Draw(*(this->sceneShader));
  
    // draw meteorites
    this->createAsteroids();
    for(unsigned int i = 0; i < this->amount; i++)
    {
        this->sceneShader->setMatrix4fv("model", glm::value_ptr(this->modelMatrices[i]));
        this->asteroid->Draw(*(this->sceneShader));
    }  

}

void SceneRenderer::showImGuiControls(SceneState &state) {
    unsigned int min = 100;
    unsigned int max = 50000;
    ImGui::SliderScalar("Asteroids", ImGuiDataType_U32, &this->amount, &min, &max, "%u");
}

void SceneRenderer::teardown()
{
}


