#include <iostream>
#include <string>
#include <array>
#include <stdexcept>
#include <random>

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
    Shader planetShader;
    Shader rockShader;

    std::vector<Shader*> postprocShaders;
    uint currentPostprocShader;
    // Textures

    // Buffers and Model

    Model planet;
    Model rock;
    uint rock_amount;

    VAO quadVAO;
    Framebuffer framebuffer;

    // Flags
    bool postprocShaderNeedsToBeChanged;
    bool objectShaderNeedsToBeChanged;

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
        // Random
        srand(glfwGetTime());

        state.cam.position += glm::vec3(0.0f, 0.0f, 3.0f);

        // Shaders
        std::string dir = "shaders/";
        planetShader = Shader(
            dir + "model_vertex.glsl",
            dir + "model_fragment.glsl"
        );
        rockShader = Shader(
            dir + "rock_vertex.glsl",
            dir + "model_fragment.glsl"
        );

        dir = "shaders/postproc/";
        postprocShaders = {
            new Shader(dir + "post2.vs", dir + "postDefault.frag"),
            new Shader(dir + "post2.vs", dir + "postInverse.frag"),
            new Shader(dir + "post2.vs", dir + "postGreyscale.frag"),
            new Shader(dir + "post2.vs", dir + "postKernel1.frag"),
            new Shader(dir + "post2.vs", dir + "postKernel2.frag")
        };
        currentPostprocShader = 0;

        // Textures

        // Buffers
        planet = Model(std::string("../models/planet/planet.obj"));
        rock = Model(std::string("../models/rock/rock.obj"));

        rock_amount = 1000;
        auto modelMatrices = generateModelMatrices();
        rock.addInstancing(*modelMatrices);
        delete modelMatrices;

        quadVAO = VAO(quad);
        framebuffer.completeGenerate(state.width,state.height);

        objectShaderNeedsToBeChanged = false;
        objectShaderNeedsToBeChanged = false;

        // Commands
        sceneCommands = {
            new KeyCommand(GLFW_KEY_C, [this]() -> void { postprocShaderNeedsToBeChanged = true; }, TOGGLE),
            new KeyCommand(GLFW_KEY_X, [this]() -> void { objectShaderNeedsToBeChanged = true; }, TOGGLE)
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
        framebuffer.resize(state.width, state.height);
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
        glm::mat4 model(1.0f);
        planetShader.use();
        planetShader.setVertexMatrices(view, model, state.projectionMatrix);
        planet.draw(planetShader);

        glm::mat4 orbit = glm::mat4(1.0f);
        float angle = glfwGetTime() * 8.0f;
        orbit = glm::rotate(orbit, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        orbit = view * orbit;

        rockShader.use();
        rockShader.setVertexMatrices(orbit, model, state.projectionMatrix);
        rock.draw(rockShader, rock_amount);
    }

    void changeShader() 
    {
        if (postprocShaderNeedsToBeChanged)
        {
            currentPostprocShader = (currentPostprocShader + 1) % postprocShaders.size();
            postprocShaderNeedsToBeChanged = false;
        }
    }

    std::vector<glm::mat4>* generateModelMatrices() {
        auto modelMatrices = new std::vector<glm::mat4>(rock_amount);
        
        float radius = 25.0;
        float offset = 8.0f;
        for(unsigned int i = 0; i < rock_amount; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float angle = (float)i / (float)rock_amount * 360.0f;
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
            (*modelMatrices)[i] = model;
        }

        return modelMatrices;
    }

};

AbstractEngine* createEngine() {
    return AbstractEngine::create<Engine>();
}