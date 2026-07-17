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
        .title = "Anti Aliasing: Green Cube Framebuffer",
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
        Shader  *sceneFBShader;
        // Helper shader structures and data
        std::unordered_map<std::string, Shader*> shaders;
        std::vector<const char*> shaderNames;
        int selectedSceneShader;
        void createShaders();
        void createBuffers();
        void setOptions();
        unsigned int cubeVAO, cubeVBO;
        glm::vec3 cubeColor = glm::vec3( 0.0f,  0.8f,  0.0f);

        // Framebuffer specific attributes and methods
        unsigned int quadVAO, quadVBO;
        unsigned int sceneFB, sceneTCB, sceneRBO;
        unsigned int interFB, interTCB, interRBO;
        void createFrameBuffers();
        void createFrameBuffer(unsigned int *, unsigned int *, unsigned int *, GLuint texture_type=GL_TEXTURE_2D, unsigned int samples = 4);
        void renderMainScene(SceneState &state);

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
    // Populate the map with all shaders
    this->shaders["Basic"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-frag.glsl");
    this->shaders["Box Blur"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-blur-frag.glsl");
    this->shaders["Gaussian Blur 3x3"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-gaussian-blur-frag.glsl");
    this->shaders["Gaussian Blur 5x5"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-big-gaussian-blur-frag.glsl");
    this->shaders["Edges"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-edge-frag.glsl");
    this->shaders["Inverse"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-inv-frag.glsl");
    this->shaders["Grayscale"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-grayscale-frag.glsl");
    this->shaders["Sharpen"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-sharpen-frag.glsl");
    this->shaders["Emboss"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-emboss-frag.glsl");
    this->shaders["Unsharp Mask"] = new Shader("shaders/quad-vertex.glsl", "shaders/quad-unsharp-mask-frag.glsl");

    // Set the main attributes
    this->sceneShader = new Shader("shaders/vertex.glsl", "shaders/frag.glsl");

    // Get the names for the ImGui interface
    for (auto& [name, shader] : this->shaders) {
        this->shaderNames.push_back(name.c_str()); 
        // Preselect shaders
        if (name == "Basic") {
            this->sceneFBShader = shader;
            this->selectedSceneShader = this->shaderNames.size() - 1;
        }
    }
}

void SceneRenderer::init()
{
    this->setOptions();
    this->createShaders();
    this->createBuffers();
    this->createFrameBuffers();
}

void SceneRenderer::createBuffers()
{
    getCubeBuffers(&this->cubeVAO, &this->cubeVBO);
    getQuadBuffers(&this->quadVAO, &this->quadVBO);
}

void SceneRenderer::renderMainScene(SceneState &state) 
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
    ImGui::Combo("Scene Shader", &this->selectedSceneShader, shaderNames.data(), shaderNames.size());
    this->sceneFBShader = this->shaders[shaderNames[selectedSceneShader]];
    ImGui::ColorEdit3("Cube Color", glm::value_ptr(this->cubeColor));
}

void SceneRenderer::createFrameBuffers()
{
    this->createFrameBuffer(&this->interFB, &this->interTCB, &this->interRBO, GL_TEXTURE_2D_MULTISAMPLE);
    this->createFrameBuffer(&this->sceneFB, &this->sceneTCB, &this->sceneRBO, GL_TEXTURE_2D);
}

void SceneRenderer::createFrameBuffer(unsigned int *fb, unsigned int *tcb, unsigned int *rbo, GLuint texture_type, unsigned int samples)
{
    // Create framebuffer and bind it
    glGenFramebuffers(1, fb);
    glBindFramebuffer(GL_FRAMEBUFFER, *fb);

    // generate texture for the framebuffer
    glGenTextures(1, tcb);
    glBindTexture(texture_type, *tcb);
    if (texture_type == GL_TEXTURE_2D) {
        glTexImage2D(texture_type, 0, GL_RGB, state.width, state.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    } else {
        glTexImage2DMultisample(texture_type, samples, GL_RGB, state.width, state.height, GL_TRUE);
    }
    glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(texture_type, 0);

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_type, *tcb, 0);

    // Generate stencil and depth testing renderbuffer
    glGenRenderbuffers(1, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, *rbo); 
    if (texture_type == GL_TEXTURE_2D) {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, state.width, state.height);
    } else {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, state.width, state.height);
    }
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach the renderbuffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *rbo);

    // Check if the framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    	std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// The full scene in this case uses a framebuffer and calls to the renderMainScene method twice
void SceneRenderer::renderScene(SceneState &state) 
{
    // Draw the main scene into the intermediate framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->interFB); 
    glClear(GL_COLOR_BUFFER_BIT);
    this->renderMainScene(state);

    // Blit the multi sampled FB into the scene FB
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->interRBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->sceneRBO);
    glBlitFramebuffer(0, 0, state.width, state.height, 0, 0, state.width, state.height, GL_COLOR_BUFFER_BIT, GL_NEAREST); 

    // Post processing of the scene
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default framebuffer
    glBindVertexArray(this->quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_DEPTH_TEST);
    glm::mat4 model = glm::mat4(1.0f);

    // Apply the scene shader to the mainScene
    glBindTexture(GL_TEXTURE_2D, this->sceneTCB);
    this->sceneFBShader->use();
    this->sceneFBShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 6);  
}


void SceneRenderer::teardown()
{
}


