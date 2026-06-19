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
    Shader renderShader;
    std::vector<Shader*> postprocShaders;
    uint currentPostprocShader;

    // Textures
    Texture2D cubeTexture;
    Texture2D floorTexture;

    // Buffers
    struct renderParams
    {
        VAO* buffer;
        glm::vec3 translate;
        // scale
        // rotate
        Texture2D* texture;
    };
    VAO cubeVAO;
    VAO planeVAO;
    std::vector<renderParams> renderVector;

    VAO quadVAO;
    Framebuffer frameFrontView;
    Framebuffer frameRearView;

    // Flags
    bool shaderNeedsToBeChanged;

    ~Engine()
    {
        for(auto ptr : basicCommands) delete ptr;
        for(auto ptr : sceneCommands) delete ptr;
        for(auto ptr : postprocShaders) delete ptr;
    }

    void sceneInit() override
    {
        // Shaders
        renderShader = Shader("shaders/shader01.vs", "shaders/shader01.frag");

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
        cubeTexture = Texture2D("../media/container.jpg", JPG, 0);
        floorTexture = Texture2D("../media/metal.png", PNG, 0);
        renderShader.use();
        renderShader.setInt("material.texture_diffuse1", 0);

        // Buffers
        cubeVAO = VAO(cube);
        planeVAO = VAO(plane);
        renderVector = {
            {&cubeVAO ,  glm::vec3(-1.0f, 0.0f, -1.0f), &cubeTexture },
            {&cubeVAO ,  glm::vec3(2.0f, 0.0f, 0.0f)  , &cubeTexture },
            {&planeVAO,  glm::vec3(0.0f)              , &floorTexture}
        };

        quadVAO = VAO(quad);
        frameFrontView.completeGenerate(state.width,state.height);
        frameRearView.completeGenerate(state.width,state.height);

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

        // Render Front View
        frameFrontView.bind();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = state.cam.lookFront();
        renderObjects(view);

        frameFrontView.unbind();

        // Render Rear View
        frameRearView.bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = state.cam.lookRearView();
        renderObjects(view);

        frameRearView.unbind();

        glDisable(GL_DEPTH_TEST);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        auto currentShader = postprocShaders[currentPostprocShader];
        currentShader->use();

        quadVAO.bind();
        glBindTexture(GL_TEXTURE_2D, frameFrontView.colorAttachment->texture);
        currentShader->setMat4("model",glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTexture(GL_TEXTURE_2D, frameRearView.colorAttachment->texture);
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.8f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        currentShader->setMat4("model",model);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        quadVAO.unbind();        
    }

    private:

    void teardown() override
    {
        cubeVAO.deleteBuffers();
        planeVAO.deleteBuffers();
        quadVAO.deleteBuffers();
        frameFrontView.deleteBuffers();
        frameRearView.deleteBuffers();
    }
    
    void renderObjects(glm::mat4 &view)
    {
        renderShader.use();
        glm::mat4 model(1.0f);
        renderShader.setVertexMatrices(view, model, state.projectionMatrix);
        
        for (auto& [vao, translate, texture] : renderVector)
        {
            vao->bind();
            texture->activate();
            model = glm::translate(glm::mat4(1.0f), translate);
            renderShader.setMat4("model", model);

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