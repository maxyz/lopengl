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
#include "lighting.hpp"

#include "basic_main.h"

class Engine : public AbstractEngine
{
public:

    // Shaders
    std::vector<Shader*> shaders;
    uint currentShader;

    // Lighting
    LightingEngine lightEngine;

    // Textures
    Texture2D woodDiff;

    // Buffers
    struct renderParams
    {
        VAO* buffer;
        // glm::vec3 translate;
        // scale
        // rotate
        AbstractTexture* texture;
        float shininess;
    };
    VAO floorVAO;
    std::vector<renderParams> renderVector;

    // VAO quadVAO;
    // Framebuffer framebuffer;

    // Flags
    // bool postprocShaderNeedsToBeChanged;
    bool objectShaderNeedsToBeChanged;
    bool antialiasing;
    bool blinn;
    bool gammaCorrection;

    ~Engine()
    {
        // Basic objects destruction
        for(auto ptr : basicCommands) delete ptr;
        for(auto ptr : sceneCommands) delete ptr;
    }

    // window with multisampling
    void initWindow() override
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 4);

        state.window = glfwCreateWindow(state.width, state.height, "CrazyWindowAction", NULL, NULL);

        if (state.window == NULL)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwSetWindowUserPointer(state.window,this);
        glfwMakeContextCurrent(state.window);
        
        glfwSetFramebufferSizeCallback(state.window, framebufferSizeCallback);
        glfwSetCursorPosCallback(state.window, mouseCallback);
        glfwSetScrollCallback(state.window, scrollCallback);

        glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        // glfwSetInputMode(state.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        glEnable(GL_MULTISAMPLE);
    }

    void sceneInit() override
    {
        state.cam.position += glm::vec3(0.0f, 0.0f, 3.0f);

        // Shaders
        std::string dir = "shaders/";
        shaders = {
            new Shader(
                dir + "A_vertex.glsl",
                dir + "A_fragment.glsl"
            )
        };
        currentShader = 0;

        // Lights
        // lightEngine.addDirectionalLight("dir1", LightingEngine::DefaultDirectionalLight());
        lightEngine.addPointLight("point1", LightingEngine::DefaultPointLight(glm::vec3(0.0f,0.5f,0.0f)));

        // Textures
        woodDiff = Texture2D("../media/wood.png", PNG);
        for (auto sh : shaders) {
            sh->use();
            sh->setInt("material.diffuse", 0);
        }
        
        // Buffers

        floorVAO = VAO(planeNormals);
        renderVector = {
            renderParams{
                &floorVAO,
                &woodDiff,
                1.0f
            }
        };

        // Flags
        objectShaderNeedsToBeChanged = false;
        antialiasing = true;
        blinn = false;
        gammaCorrection = true;

        // Commands
        sceneCommands = {
            // new KeyCommand(GLFW_KEY_C, [this]() -> void { postprocShaderNeedsToBeChanged = true; }, TOGGLE),
            new KeyCommand(GLFW_KEY_X, [this]() -> void { objectShaderNeedsToBeChanged = true; }, TOGGLE),
            new KeyCommand(GLFW_KEY_V, [this]() -> void { antialiasing = !antialiasing; }, TOGGLE),
            new KeyCommand(GLFW_KEY_B, [this]() -> void { blinn = !blinn; }, TOGGLE),
            new KeyCommand(GLFW_KEY_G, [this]() -> void { gammaCorrection = !gammaCorrection; }, TOGGLE)
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

        if (antialiasing)
        {
            glEnable(GL_MULTISAMPLE);
        } else {
            glDisable(GL_MULTISAMPLE);
        }

        if (gammaCorrection)
        {
            glEnable(GL_FRAMEBUFFER_SRGB);
        } else {
            glDisable(GL_FRAMEBUFFER_SRGB);
        }
        
        
    }

    // MAIN SCENE SPECIFIC METHOD
    void renderScene() override
    {
        glEnable(GL_DEPTH_TEST);

        // framebuffer.bind();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render objects
        glm::mat4 view = state.cam.lookFront();
        renderObjects(view);
    }

    private:

    void teardown() override 
    // Teardown no sirve mucho porque opengl cuando termina borra todo automaticamente.
    // A lo sumo serviría si tengo que destruir todo lo del engine en medio de la ejecución.
    {
        floorVAO.deleteBuffers();
        woodDiff.deleteTexture();
        for (auto sh : shaders) delete sh;
    }

    void renderObjects(glm::mat4 &view)
    {
        auto shader = shaders[currentShader];
        shader->use();
        shader->setBool("blinn", blinn);
        shader->setVec3("viewPos", state.cam.position);

        lightEngine.sendToShader(*shader);

        glm::mat4 model(1.0f);
        shaders[currentShader]->setVertexMatrices(view, model, state.projectionMatrix);

        for (auto& [vao, diff, shin] : renderVector)
        {
            vao->bind();
            diff->activate();
            shader->setFloat("material.shininess", shin);

            glDrawArrays(GL_TRIANGLES, 0, vao->renderVertices);
        }   
    }

    void changeShader() 
    {
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