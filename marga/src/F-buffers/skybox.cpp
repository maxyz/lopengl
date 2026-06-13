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
        .title = "Skybox",
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
        Shader  *sourceShader;
        Shader  *sceneFBShader;
        Shader  *mirrorShader;
        Shader  *skyboxShader;
        // Helper shader structures and data
        std::unordered_map<std::string, Shader*> shaders;
        std::vector<const char*> shaderNames;
        int selectedSceneShader, selectedMirrorShader;
        void createShaders();

        CubeTexture *skybox;
        Texture *cubeMaterial;
        Texture *floorMaterial;
        LightSet *lights;
        unsigned int skyboxVAO, skyboxVBO;
        unsigned int cubeVAO, cubeVBO;
        unsigned int planeVAO, planeVBO;
        unsigned int quadVAO, quadVBO;
        unsigned int lightVAO;

        void setOptions();

        // Framebuffer specific attributes and methods
        unsigned int sceneFB, sceneTCB, sceneRBO;
        unsigned int mirrorFB, mirrorTCB, mirrorRBO;
        void createFrameBuffers();
        void createFrameBuffer(unsigned int *, unsigned int *, unsigned int *);
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
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    // Configure depth testing
    glDepthFunc(GL_LEQUAL);
    // Enable blending
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Enable Face Culling (drop backward faces)
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); 
    
    // Wireframe mode
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
    this->sourceShader = new Shader("shaders/source-vertex.glsl", "shaders/source-frag.glsl");
    this->sceneShader = new Shader("shaders/reflection-vertex.glsl", "shaders/reflection-frag.glsl");
    this->skyboxShader = new Shader("shaders/skybox-vertex.glsl", "shaders/skybox-frag.glsl");

    // Get the names for the ImGui interface
    for (auto& [name, shader] : this->shaders) {
        this->shaderNames.push_back(name.c_str()); 
        // Preselect shaders
        if (name == "Basic") {
            this->sceneFBShader = shader;
            this->selectedSceneShader = this->shaderNames.size() - 1;
        }
        if (name == "Box Blur") {
            this->mirrorShader = shader;
            this->selectedMirrorShader = this->shaderNames.size() - 1;
        }
    }
}

void SceneRenderer::init()
{
    this->setOptions();
    this->createShaders();

    this->mirrorShader->use();
    this->mirrorShader->setInt("screenTexture", 0);

    //Texture::flip_vertically();
    this->floorMaterial = new Texture("../media/marble.jpg");
    this->cubeMaterial = new Texture("../media/container.jpg");

    std::vector<std::string> faces = {
        "../media/skybox/right.jpg",
        "../media/skybox/left.jpg",
        "../media/skybox/top.jpg",
        "../media/skybox/bottom.jpg",
        "../media/skybox/front.jpg",
        "../media/skybox/back.jpg"
    };
    this->skybox = new CubeTexture(faces);

    /*Texture grass = Texture("../media/grass.png", GL_RGBA);
    grass.set_wrap(GL_CLAMP_TO_EDGE);

    Texture windowPane = Texture("../media/blending_transparent_window.png", GL_RGBA);
    Texture windowSpecular = Texture("../media/transparent_window_specular.png", GL_RGBA);
    windowPane.set_wrap(GL_CLAMP_TO_EDGE);*/
    
    this->sceneShader->use();
    this->sceneShader->setInt("material.texture_diffuse1", 0);
    this->sceneShader->setInt("material.texture_specular1", 1);
    this->sceneShader->setInt("skybox", 1);

    this->skyboxShader->use();
    this->skyboxShader->setInt("skybox", 1);
    // VAOs and VBOs
    getCubeBuffers(&this->cubeVAO, &this->cubeVBO);
    getPlaneBuffers(&this->planeVAO, &this->planeVBO);
    getQuadBuffers(&this->quadVAO, &this->quadVBO);
    getLightBuffers(&this->lightVAO, this->cubeVBO);
    getSkyboxBuffers(&this->skyboxVAO, &this->skyboxVBO);

    // Starting lighting values (position, color, ambient, diffuse, specular, constant, linear, quadratic, cutoff)
    DirectionalLight directionalLight(glm::vec3(-0.2f, 1.0f, 0.3f));
    SpotLight spotLight;
    std::array<PositionalLight, 4> positionalLights = {{
    	PositionalLight(glm::vec3( 1.7f,  1.2f,  2.0f), glm::vec3( 1.0f,  1.0f,  1.0f), 0.2f, 0.5f),
	    PositionalLight(glm::vec3( 4.3f, -3.3f, -4.0f), glm::vec3( 0.0f,  0.0f,  1.0f), 0.2f, 0.5f),
    	PositionalLight(glm::vec3(-4.0f,  2.0f, -2.0f), glm::vec3( 1.0f,  0.0f,  0.0f), 0.2f, 0.5f),
    	PositionalLight(glm::vec3( 1.0f,  0.0f, -3.0f), glm::vec3( 0.0f,  1.0f,  0.0f), 0.2f, 0.5f)
    }};
    this->lights = new LightSet(directionalLight, spotLight, 4, positionalLights);

    // Transparent objects - not in use
    /*std::vector<glm::vec3> transparentObjects;
    transparentObjects.push_back(glm::vec3(-1.5f,  0.0f, -0.48f));
    transparentObjects.push_back(glm::vec3( 1.5f,  0.0f,  0.51f));
    transparentObjects.push_back(glm::vec3( 0.0f,  0.0f,  0.7f));
    transparentObjects.push_back(glm::vec3(-0.3f,  0.0f, -2.3f));
    transparentObjects.push_back(glm::vec3( 0.5f,  0.0f, -0.6f));  

    std::multimap<float, glm::vec3> sorted;
    sortTransparent(true, transparentObjects, &sorted);*/

    this->createFrameBuffers();
}

void SceneRenderer::renderMainScene(SceneState &state) 
{
    glClearColor(state.bgColor.x, state.bgColor.y, state.bgColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Coordinate matrixes
    // ** View **
    glm::mat4 view = state.camera.GetViewMatrix(); // Full view
    glm::mat4 skyboxView = glm::mat4(glm::mat3(state.camera.GetViewMatrix())); // View without translation
    // ** Projection **
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(state.camera.Zoom), state.width/state.height, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST);

    // Now the rest of the scene 
    this->sceneShader->use();
    this->sceneShader->setVec3f("viewPos", glm::value_ptr(state.camera.Position));
    this->sceneShader->setFloat("material.shininess", state.shininess);

    glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox->ID);
    lights = this->lights;
    lights->directionalLight.setShaderValues(*this->sceneShader, "dirLight");
    for (int i = 0; i < lights->positionalLightAmount; i++) {
        lights->positionalLights[i].setShaderValues(*this->sceneShader, std::format("pointLights[{}]", i));
    }
    lights->spotLight.position = state.camera.Position;
    lights->spotLight.direction = state.camera.Front;
    lights->spotLight.setShaderValues(*this->sceneShader, "spotLight");

    this->sceneShader->setMatrix4fv("view", glm::value_ptr(view));
    this->sceneShader->setMatrix4fv("projection", glm::value_ptr(projection));

    glm::mat4 model = glm::mat4(1.0f);

    // Draw 2 cubes
    glBindVertexArray(this->cubeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->cubeMaterial->ID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->cubeMaterial->ID);
    model = glm::translate(model, glm::vec3(-1.5f, 0.0f, -1.5f));
    this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.5f, 0.0f, -0.5f));
    this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // Draw the lights on the screen
    this->sourceShader->use();
    this->sourceShader->setMatrix4fv("view", glm::value_ptr(view));
    this->sourceShader->setMatrix4fv("projection", glm::value_ptr(projection));

    for (int i = 0; i < lights->positionalLightAmount; i++) {
        PositionalLight light = lights->positionalLights[i];
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, light.position);
        model = glm::scale(model, glm::vec3(0.2f));
        this->sourceShader->setMatrix4fv("model", glm::value_ptr(model));
        this->sourceShader->setVec3f("lightColor", glm::value_ptr(light.color));
        // draw the light cube object
        glBindVertexArray(this->lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // The floor and the transparent objects are not culled
    glDisable(GL_CULL_FACE);

    this->sceneShader->use();
    // floor
    glBindVertexArray(this->planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->floorMaterial->ID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, this->floorMaterial->ID);
    model = glm::mat4(1.0f);
    this->sceneShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Put the transparent objects into a map to get them sorted by distance
    /*
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, windowPane.ID);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, windowSpecular.ID);
    sortTransparent(recalculateObjects, transparentObjects, &sorted);
    for(std::map<float,glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) 
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, it->second);				
        this->sceneShader.setMatrix4fv("model", glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }*/
    glEnable(GL_CULL_FACE);

     // The skybox goes at the end, with depth testing enabled
    this->skyboxShader->use();
    this->skyboxShader->setMatrix4fv("view", glm::value_ptr(skyboxView));
    this->skyboxShader->setMatrix4fv("projection", glm::value_ptr(projection));
    glBindVertexArray(this->skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox->ID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
   
}

void SceneRenderer::showImGuiControls(SceneState &state) {
    this->lights->showImGuiControls(state);

    // Shader selector
    ImGui::Combo("Scene Shader", &this->selectedSceneShader, shaderNames.data(), shaderNames.size());
    ImGui::Combo("Mirror Shader", &this->selectedMirrorShader, shaderNames.data(), shaderNames.size());
    this->sceneFBShader = this->shaders[shaderNames[selectedSceneShader]];
    this->mirrorShader = this->shaders[shaderNames[selectedMirrorShader]];
}

void SceneRenderer::createFrameBuffers()
{
    this->createFrameBuffer(&this->sceneFB, &this->sceneTCB, &this->sceneRBO);
    this->createFrameBuffer(&this->mirrorFB, &this->mirrorTCB, &this->mirrorRBO);
}

void SceneRenderer::createFrameBuffer(unsigned int *fb, unsigned int *tcb, unsigned int *rbo)
{
    // Create framebuffer and bind it
    glGenFramebuffers(1, fb);
    glBindFramebuffer(GL_FRAMEBUFFER, *fb);

    // generate texture for the framebuffer
    glGenTextures(1, tcb);
    glBindTexture(GL_TEXTURE_2D, *tcb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state.width, state.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tcb, 0);

    // Generate stencil and depth testing renderbuffer
    glGenRenderbuffers(1, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, *rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, state.width, state.height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach the renderbuffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *rbo);

    // Check if the framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    	std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// The full scene in this case uses a framebuffer and calls to the renderMainScene method twice
void SceneRenderer::renderScene(SceneState &state) 
{
    // Draw to the mirror framebuffer with the inverted camera and no spotlight
    glBindFramebuffer(GL_FRAMEBUFFER, this->mirrorFB);
    state.camera.setRearView();
    bool spotlightActive = this->lights->spotLight.active;
    this->lights->spotLight.active = false;
    this->renderMainScene(state);
    this->lights->spotLight.active = spotlightActive;
    state.camera.unsetRearView();

    // Draw the main scene with the restored camera into the scene framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->sceneFB); 
    glClear(GL_COLOR_BUFFER_BIT);
    this->renderMainScene(state);

    // Use the default framebuffer to render the final scene
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

    // And now draw the background mirror
    glBindTexture(GL_TEXTURE_2D, this->mirrorTCB);
    this->mirrorShader->use();  
    model = glm::translate(model, glm::vec3(0.0f, 0.8f, 0.0f));
    model = glm::scale(model, glm::vec3(0.2f));
    this->mirrorShader->setMatrix4fv("model", glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 6);  
}

void SceneRenderer::teardown()
{
    // Clean up buffers
    glDeleteVertexArrays(1, &this->cubeVAO);
    glDeleteVertexArrays(1, &this->planeVAO);
    glDeleteVertexArrays(1, &this->lightVAO);
    glDeleteVertexArrays(1, &this->quadVAO);
    glDeleteBuffers(1, &this->cubeVBO);
    glDeleteBuffers(1, &this->planeVBO);
    glDeleteBuffers(1, &this->quadVBO);
    glDeleteFramebuffers(1, &this->mirrorFB);
    glDeleteFramebuffers(1, &this->sceneFB);
}


