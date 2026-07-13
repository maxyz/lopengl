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
        .title = "Asteroids",
        .bgColor = glm::vec3( 0.1f,  0.1f,  0.4f),
        .camera = Camera(glm::vec3(0.0f, 15.0f, 100.0f)),
        .lastX = 400,
        .lastY = 300,
        .firstMouse = true,
        .shininess = 32.0,
};

class SceneRenderer: public AbstractSceneRenderer {
    private:
        // Main Shaders
        Shader  *sceneShader;
        Shader  *instancedShader;
        // Helper shader structures and data
        void createShaders();

        void setOptions();

        Model *planet;
        Model *asteroid;
        glm::mat4 *modelMatrices;
        unsigned int amount = 1000;
        void createAsteroids();
        unsigned int lastIndex = 0;
        float orbitSpeed = 5;

        // Used for instanced Arrays
        unsigned int instanceVBO;
        int shaderToUse = 1;
        void createBuffers();

        // Adding a Skybox
        Shader  *skyboxShader;
        CubeTexture *skybox;
        unsigned int skyboxVAO, skyboxVBO;
        void createSkybox();
        void renderSkybox(SceneState &state);
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
    glDepthFunc(GL_LEQUAL);
}

void SceneRenderer::createShaders()
{
    this->sceneShader = new Shader("shaders/simple-model-vertex.glsl", "shaders/model-frag.glsl");
    this->instancedShader = new Shader("shaders/instanced-model-vertex.glsl", "shaders/model-frag.glsl");
    this->instancedShader->setInt("material.texture_diffuse1", 0);
}

void SceneRenderer::init()
{
    this->setOptions();
    this->createShaders();
    this->planet = new Model("../media/models/planet.obj");
    this->asteroid = new Model("../media/models/rock.obj");
    state.camera.MovementSpeed = 40.0;
    this->createBuffers();
    this->createAsteroids();
    this->createSkybox();
}

void SceneRenderer::createSkybox()
{
    getSkyboxBuffers(&this->skyboxVAO, &this->skyboxVBO);
    std::vector<std::string> faces = {
        "../media/spaceskybox/right.png",
        "../media/spaceskybox/left.png",
        "../media/spaceskybox/top.png",
        "../media/spaceskybox/bottom.png",
        "../media/spaceskybox/front.png",
        "../media/spaceskybox/back.png"
    };
    this->skybox = new CubeTexture(faces);
    this->skyboxShader = new Shader("shaders/skybox-vertex.glsl", "shaders/skybox-frag.glsl");
    this->skyboxShader->use();
    // This number corresponds to the active texture at the time of drawing the SkyBox
    // So, if something else is changing the active texture, we need to do:
    // glActiveTexture(GL_TEXTURE0);
    // Before binding the skybox texture
    this->skyboxShader->setInt("skybox", 0);
}

void SceneRenderer::renderSkybox(SceneState &state)
{
    glm::mat4 skyboxView = glm::mat4(glm::mat3(state.camera.GetViewMatrix())); // View without translation
    glm::mat4 projection = glm::perspective(glm::radians(state.camera.Zoom), state.width/state.height, 0.1f, 1000.0f);
    this->skyboxShader->use();
    this->skyboxShader->setMatrix4fv("view", glm::value_ptr(skyboxView));
    this->skyboxShader->setMatrix4fv("projection", glm::value_ptr(projection));
    glBindVertexArray(this->skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox->ID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void SceneRenderer::createBuffers()
{
    // vertex buffer object
    glGenBuffers(1, &this->instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    
    // Bind the modelMatrix vertex attribute to each rock's mesh
    for(unsigned int i = 0; i < this->asteroid->meshes.size(); i++)
    {
        unsigned int VAO = this->asteroid->meshes[i].VAO;
        glBindVertexArray(VAO);
        // vertex attributes
        std::size_t vec4Size = sizeof(glm::vec4);
        glEnableVertexAttribArray(3); 
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
        glEnableVertexAttribArray(4); 
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
        glEnableVertexAttribArray(5); 
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
        glEnableVertexAttribArray(6); 
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
    
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
    
        glBindVertexArray(0);
    } 
}

void SceneRenderer::createAsteroids()
{
    if (this->lastIndex == this->amount) return;
    if (this->lastIndex > 0) {
        delete this->modelMatrices;
    }
    this->modelMatrices = new glm::mat4[amount];
    srand(glfwGetTime()); // initialize random seed	
    float radius = 50.0f;
    float offset = 8.0f;
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

    // Recopy the data for the instanced arrays case
    glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, this->amount * sizeof(glm::mat4), &this->modelMatrices[0], GL_STATIC_DRAW);
}

void SceneRenderer::renderScene(SceneState &state) 
{
    glClearColor(state.bgColor.x, state.bgColor.y, state.bgColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = state.camera.GetViewMatrix(); // Full view
    glm::mat4 projection = glm::perspective(glm::radians(state.camera.Zoom), state.width/state.height, 0.1f, 1000.0f);

    // draw the planet
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
    this->sceneShader->use();
    this->sceneShader->setMatrix4fv("view", glm::value_ptr(view));
    this->sceneShader->setMatrix4fv("projection", glm::value_ptr(projection));
    this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
    this->planet->Draw(*(this->sceneShader));

    // Calculate the rotation in the orbit for the meteorites
    glm::mat4 orbit = glm::mat4(1.0f);
    float angle = glfwGetTime() * this->orbitSpeed;
    orbit = glm::rotate(orbit, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
  
    // draw meteorites
    this->createAsteroids();
    if (this->shaderToUse == 0) {
        orbit = view * orbit;
        this->sceneShader->setMatrix4fv("view", glm::value_ptr(orbit));
        for(unsigned int i = 0; i < this->amount; i++)
        {
            this->sceneShader->setMatrix4fv("model", glm::value_ptr(this->modelMatrices[i]));
            this->asteroid->Draw(*(this->sceneShader));
        }
    } else {
        
        glm::mat4 pvo = projection * view * orbit;

        this->instancedShader->use();
        this->instancedShader->setMatrix4fv("projection", glm::value_ptr(pvo));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->asteroid->textures_loaded[0].ID); // note: we also made the textures_loaded vector public (instead of private) from the model class.
        for(unsigned int i = 0; i < this->asteroid->meshes.size(); i++)
        {
            glBindVertexArray(this->asteroid->meshes[i].VAO);
            glDrawElementsInstanced(
                GL_TRIANGLES, this->asteroid->meshes[i].indices.size(), GL_UNSIGNED_INT, 0, this->amount
            );
        }
    } 

    // The skybox goes at the end, with depth testing enabled
    this->renderSkybox(state);
}

void SceneRenderer::showImGuiControls(SceneState &state) {
    unsigned int min = 100;
    unsigned int max = 100000;
    ImGui::SliderScalar("Asteroids", ImGuiDataType_U32, &this->amount, &min, &max, "%u");
    ImGui::SliderFloat("Orbit Speed", &this->orbitSpeed, 0, 20);
    static const char* items[] = {"Old school", "Instanced Arrays"};
    ImGui::Combo("Instancing mode", &this->shaderToUse, items, 2);
}

void SceneRenderer::teardown()
{
}


