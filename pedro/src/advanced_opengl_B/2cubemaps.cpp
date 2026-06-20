#include <iostream>
#include <string>
#include <array>
#include <stdexcept>

#include "shader.h"
#include "model.h"
#include "camera.h"
#include "texture.h"
#include "key_command.h"
#include "../models/vertices/object_vertices.cpp"
#include "buffers.h"

#include "basic_main.h"

class Engine : public AbstractEngine
{
public:

    // Shaders
    Shader objectRenderShader;
    Shader skyboxRenderShader;

    std::vector<Shader*> postprocShaders;
    uint currentPostprocShader;

    // Textures
    Texture2D cubeTexture;
    Cubemap skyboxTexture;

    // Buffers
    struct renderParams
    {
        VAO* buffer;
        glm::vec3 translate;
        // scale
        // rotate
        AbstractTexture* texture;
    };
    VAO cubeVAO;
    VAO skyboxVAO;
    std::vector<renderParams> renderVector;

    VAO quadVAO;
    Framebuffer frameFrontView;

    // Flags
    bool shaderNeedsToBeChanged;

    ~Engine()
    {
        // Basic objects destruction
        for(auto ptr : basicCommands) delete ptr;
        for(auto ptr : sceneCommands) delete ptr;

        // Scene objects destruction
        for(auto ptr : postprocShaders) delete ptr;
    }

    void sceneInit() override
    {
        // Shaders
        objectRenderShader = Shader("shaders/cubemaps1.vs", "shaders/cubemaps1.frag");
        skyboxRenderShader = Shader("shaders/skybox.vs", "shaders/skybox.frag");

        std::string dir = "shaders/postproc/";
        postprocShaders = {
            new Shader(dir + "post2.vs", dir + "postDefault.frag"),
            new Shader(dir + "post2.vs", dir + "postInverse.frag"),
            new Shader(dir + "post2.vs", dir + "postGreyscale.frag"),
            new Shader(dir + "post2.vs", dir + "postKernel1.frag"),
            new Shader(dir + "post2.vs", dir + "postKernel2.frag")
        };
        currentPostprocShader = 0;

        // Textures
        dir = "../media/skybox/";
        std::vector<std::string> faces =
        {
            dir + "right.jpg",
            dir + "left.jpg",
            dir + "top.jpg",
            dir + "bottom.jpg",
            dir + "front.jpg",
            dir + "back.jpg"
        };
        skyboxTexture = Cubemap(faces, 0);
        skyboxRenderShader.use();
        skyboxRenderShader.setInt("skybox", 0);

        cubeTexture = Texture2D("../media/container.jpg", JPG, 0);
        objectRenderShader.use();
        objectRenderShader.setInt("material.texture_diffuse1", 0);

        // Buffers
        cubeVAO = VAO(cube);
        skyboxVAO = VAO(skybox);
        renderVector = {
            {&cubeVAO  ,  glm::vec3(-1.0f, 0.0f, -1.0f), &cubeTexture},
            {&cubeVAO  ,  glm::vec3(2.0f, 0.0f, 0.0f)  , &cubeTexture}
        };

        quadVAO = VAO(quad);
        frameFrontView.completeGenerate(state.width,state.height);

        // Commands
        sceneCommands = {
            new KeyCommand(GLFW_KEY_C, [this]() -> void { shaderNeedsToBeChanged = true; }, TOGGLE)
        };

        // Parameters
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

    void update() override
    {
        updateFrames();
        processInput();
        changeShader();
        updateProjection();
    }

    // MAIN SCENE SPECIFIC METHOD
    void renderScene() override
    {
        glEnable(GL_DEPTH_TEST);

        // // Render Front View
        frameFrontView.bind();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render objects
        glm::mat4 view = state.cam.lookFront();
        renderObjects(view);
        
        // Render skybox;
        glDepthFunc(GL_LEQUAL);
        glm::mat4 noTranslationView = glm::mat4(glm::mat3(view));
        renderSkybox(noTranslationView);
        glDepthFunc(GL_LESS);

        frameFrontView.unbind();
        
        // Draw the Frame on screen
        glDisable(GL_DEPTH_TEST);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        auto currentShader = postprocShaders[currentPostprocShader];
        currentShader->use();

        quadVAO.bind();
        glBindTexture(GL_TEXTURE_2D, frameFrontView.colorAttachment->texture);
        currentShader->setMat4("model",glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        quadVAO.unbind();        
    }

    private:

    void teardown() override 
    // Teardown no sirve mucho porque opengl cuando termina borra todo automaticamente.
    // A lo sumo serviría si tengo que destruir todo lo del engine en medio de la ejecución.
    {
        cubeVAO.deleteBuffers();
        skyboxVAO.deleteBuffers();
        quadVAO.deleteBuffers();
        frameFrontView.deleteBuffers();
        cubeTexture.deleteTexture();
        skyboxTexture.deleteTexture();
    }
    
    void renderSkybox(glm::mat4 &view) {
        skyboxRenderShader.use();
        glm::mat4 model(1.0f);
        skyboxRenderShader.setVertexMatrices(view, model, state.projectionMatrix);

        skyboxVAO.bind();
        skyboxTexture.activate();

        glDrawArrays(GL_TRIANGLES, 0, skyboxVAO.renderVertices);
    }

    void renderObjects(glm::mat4 &view)
    {
        objectRenderShader.use();
        glm::mat4 model(1.0f);
        objectRenderShader.setVertexMatrices(view, model, state.projectionMatrix);
        
        for (auto& [vao, translate, texture] : renderVector)
        {
            vao->bind();
            texture->activate(0);
            model = glm::translate(glm::mat4(1.0f), translate);
            objectRenderShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, vao->renderVertices);
        }
    }

    void changeShader() 
    {
        if (shaderNeedsToBeChanged)
        {
            currentPostprocShader = (currentPostprocShader + 1) % postprocShaders.size();
            shaderNeedsToBeChanged = false;
        }
    }

};

AbstractEngine* createEngine() {
    return AbstractEngine::create<Engine>();
}