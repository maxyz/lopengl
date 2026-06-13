#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <map>
#include <format>
#include <cmath>
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
        .title = "Rear view mirror",
        .bgColor = glm::vec3( 0.2f,  0.3f,  0.5f),
        .camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f)),
        .lastX = 400,
        .lastY = 300,
        .firstMouse = true,
        .shininess = 32.0,
};

class SceneRenderer: public AbstractSceneRenderer {
    private:
        Shader  *sceneShader;
        Shader  *sourceShader;
        Shader  *quadShader;
        Texture *cubeMaterial;
        Texture *floorMaterial;
        LightSet *lights;
        unsigned int cubeVAO, cubeVBO;
        unsigned int planeVAO, planeVBO;
        unsigned int quadVAO, quadVBO;
        unsigned int lightVAO;

        void setOptions();

        // Framebuffer specific attributes and methods
        unsigned int framebuffer, textureColorbuffer, rbo;
        void createFrameBuffer();
        void renderMainScene(SceneState state);

    public:
        SceneRenderer() {}
        void init();
        void renderScene(SceneState state);
        void showImGuiControls(SceneState state);
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
    glDepthFunc(GL_LESS);
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

void SceneRenderer::init()
{
    this->setOptions();

    // Read shaders
    this->sceneShader = new Shader("shaders/vertex.glsl", "shaders/textured-multi-lights.glsl");
    this->sourceShader = new Shader("shaders/source-vertex.glsl", "shaders/source-frag.glsl");
    this->quadShader = new Shader("shaders/quad-vertex.glsl", "shaders/quad-edge-frag.glsl");

    this->quadShader->use();
    this->quadShader->setInt("screenTexture", 0);

    Texture::flip_vertically();
    this->floorMaterial = new Texture("../media/marble.jpg", GL_RGB);
    this->cubeMaterial = new Texture("../media/container.jpg", GL_RGB);

    /*Texture grass = Texture("../media/grass.png", GL_RGBA);
    grass.set_wrap(GL_CLAMP_TO_EDGE);

    Texture windowPane = Texture("../media/blending_transparent_window.png", GL_RGBA);
    Texture windowSpecular = Texture("../media/transparent_window_specular.png", GL_RGBA);
    windowPane.set_wrap(GL_CLAMP_TO_EDGE);*/
    
    this->sceneShader->use();
    this->sceneShader->setInt("material.texture_diffuse1", 0);
    this->sceneShader->setInt("material.texture_specular1", 1);

    // VAOs and VBOs
    getCubeBuffers(&this->cubeVAO, &this->cubeVBO);
    getPlaneBuffers(&this->planeVAO, &this->planeVBO);
    getQuadBuffers(&this->quadVAO, &this->quadVBO);
    getLightBuffers(&this->lightVAO, this->cubeVBO);

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

    this->createFrameBuffer();
}

void SceneRenderer::renderMainScene(SceneState state) 
{
    glEnable(GL_DEPTH_TEST);

    // rendering commands here
    glClearColor(state.bgColor.x, state.bgColor.y, state.bgColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->sceneShader->use();
    this->sceneShader->setVec3f("viewPos", glm::value_ptr(state.camera.Position));
    this->sceneShader->setFloat("material.shininess", state.shininess);

    lights = this->lights;
    lights->directionalLight.setShaderValues(*this->sceneShader, "dirLight");

    for (int i = 0; i < lights->positionalLightAmount; i++) {
        lights->positionalLights[i].setShaderValues(*this->sceneShader, std::format("pointLights[{}]", i));
    }

    lights->spotLight.position = state.camera.Position;
    lights->spotLight.direction = state.camera.Front;
    lights->spotLight.setShaderValues(*this->sceneShader, "spotLight");

    // Coordinate matrixes
    // ** View **
    glm::mat4 view = state.camera.GetViewMatrix();
    // ** Projection **
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(state.camera.Zoom), state.width/state.height, 0.1f, 100.0f);

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
}

void SceneRenderer::showImGuiControls(SceneState state) {
    lights->showImGuiControls(state);
    
}


void SceneRenderer::createFrameBuffer()
{
    // Create framebuffer and bind it
    glGenFramebuffers(1, &this->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // generate texture for the framebuffer
    glGenTextures(1, &this->textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, this->textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state.width, state.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach it to currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->textureColorbuffer, 0);

    // Generate stencil and depth testing renderbuffer
    glGenRenderbuffers(1, &this->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, this->rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, state.width, state.height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach the renderbuffer to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rbo);

    // Check if the framebuffer is complete
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    	std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// The full scene in this case uses a framebuffer and calls to the renderMainScene method twice
void SceneRenderer::renderScene(SceneState state) 
{
    // first pass: draw to the framebuffer with the inverted camera
    glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);
    state.camera.setRearView();
    this->renderMainScene(state);
    state.camera.unsetRearView();

    // second pass: draw the main scene with the restored camera
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default framebuffer
    glClear(GL_COLOR_BUFFER_BIT);
    this->renderMainScene(state);

    // And now draw the background mirror
    this->quadShader->use();  
    glBindVertexArray(this->quadVAO);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->textureColorbuffer);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.8f, 0.0f));
    model = glm::scale(model, glm::vec3(0.2f));
    this->quadShader->setMatrix4fv("model", glm::value_ptr(model));
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
    glDeleteFramebuffers(1, &this->framebuffer);
}


