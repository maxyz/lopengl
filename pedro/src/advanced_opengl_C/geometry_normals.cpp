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
    Shader normalShader;
    std::vector<Shader*> postprocShaders;
    std::vector<Shader*> shaders;
    uint currentPostprocShader;
    uint currentShader;

    // Textures

    // Buffers and Model

    Model backpack;

    VAO quadVAO;
    Framebuffer framebuffer;

    // Flags
    bool postprocShaderNeedsToBeChanged;
    bool objectShaderNeedsToBeChanged;
    bool showNormals;

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
        state.cam.position += glm::vec3(0.0f, 0.0f, 3.0f);

        // Shaders
        std::string dir = "shaders/";
        normalShader = Shader(
            dir + "normal_vertex.glsl",
            dir + "normal_fragment.glsl",
            dir + "normal_geometry.glsl"
        );
        shaders = {
            new Shader(
                dir + "model_vertex.glsl", 
                dir + "model_fragment.glsl"
            )
        };

        dir = "shaders/postproc/";
        postprocShaders = {
            new Shader(dir + "post2.vs", dir + "postDefault.frag"),
            new Shader(dir + "post2.vs", dir + "postInverse.frag"),
            new Shader(dir + "post2.vs", dir + "postGreyscale.frag"),
            new Shader(dir + "post2.vs", dir + "postKernel1.frag"),
            new Shader(dir + "post2.vs", dir + "postKernel2.frag")
        };
        currentPostprocShader = 0;
        currentShader = 0;

        // Textures

        // Buffers
        VertexVector points
        (
            new std::vector<float>{
                -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
                0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
                0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
                -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
            },
            AttribInfo
            {
                VertexAttribInfo{0, 2, 5, 0},
                VertexAttribInfo{1, 3, 5, 2}
            },
            4
        );

        backpack = Model(std::string("../models/backpack/backpack.obj"));

        quadVAO = VAO(quad);
        framebuffer.completeGenerate(state.width,state.height);

        objectShaderNeedsToBeChanged = false;
        objectShaderNeedsToBeChanged = false;
        showNormals = false;

        // Commands
        sceneCommands = {
            new KeyCommand(GLFW_KEY_C, [this]() -> void { postprocShaderNeedsToBeChanged = true; }, TOGGLE),
            new KeyCommand(GLFW_KEY_X, [this]() -> void { objectShaderNeedsToBeChanged = true; }, TOGGLE),
            new KeyCommand(GLFW_KEY_N, [this]() -> void { showNormals = !showNormals; }, TOGGLE)
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
        framebuffer.bind();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render objects
        glm::mat4 view = state.cam.lookFront();
        renderObjects(view);

        framebuffer.unbind();
        
        // Draw the Frame on screen
        glDisable(GL_DEPTH_TEST);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        auto currentShader = postprocShaders[currentPostprocShader];
        currentShader->use();

        quadVAO.bind(); 
        glBindTexture(GL_TEXTURE_2D, framebuffer.colorAttachment->texture);
        currentShader->setMat4("model",glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        quadVAO.unbind();
    }

    private:

    void teardown() override 
    // Teardown no sirve mucho porque opengl cuando termina borra todo automaticamente.
    // A lo sumo serviría si tengo que destruir todo lo del engine en medio de la ejecución.
    {
        framebuffer.deleteBuffers();
    }

    void renderObjects(glm::mat4 &view)
    {
        shaders[currentShader]->use();

        glm::mat4 model(1.0f);
        shaders[currentShader]->setVertexMatrices(view, model, state.projectionMatrix);
        backpack.draw(*shaders[currentShader]);

        if (showNormals)
        {
            normalShader.use();
            normalShader.setVertexMatrices(view, model, state.projectionMatrix);
            backpack.draw(normalShader);
        }        
    }

    void changeShader() 
    {
        if (postprocShaderNeedsToBeChanged)
        {
            currentPostprocShader = (currentPostprocShader + 1) % postprocShaders.size();
            postprocShaderNeedsToBeChanged = false;
        }
        if (objectShaderNeedsToBeChanged)
        {
            currentShader = (currentShader + 1) % shaders.size();
            objectShaderNeedsToBeChanged = false;
        }
    }

};

AbstractEngine* createEngine() {
    return AbstractEngine::create<Engine>();
}