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
    // std::vector<Shader*> postprocShaders;
    std::vector<Shader*> shaders;
    uint currentShader;
    // uint currentPostprocShader;

    // Textures

    // Buffers
    // struct renderParams
    // {
    //     VAO* buffer;
    //     // glm::vec3 translate;
    //     // scale
    //     // rotate
    //     // AbstractTexture* texture;
    //     glm::vec3 color;
    // };
    VAO cubeVAO;
    // std::vector<renderParams> renderVector;

    // VAO quadVAO;
    // Framebuffer framebuffer;

    // Flags
    // bool postprocShaderNeedsToBeChanged;
    bool objectShaderNeedsToBeChanged;
    bool antialiasing;

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
                dir + "antialiasingVertex.glsl", 
                dir + "antialiasingFragment.glsl"
            )
        };
        currentShader = 0;

        // dir = "shaders/postproc/";
        // postprocShaders = {
        //     new Shader(dir + "post2.vs", dir + "postDefault.frag"),
        //     new Shader(dir + "post2.vs", dir + "postInverse.frag"),
        //     new Shader(dir + "post2.vs", dir + "postGreyscale.frag"),
        //     new Shader(dir + "post2.vs", dir + "postKernel1.frag"),
        //     new Shader(dir + "post2.vs", dir + "postKernel2.frag")
        // };
        // currentPostprocShader = 0;

        // Textures

        // Buffers

        cubeVAO = VAO(cube);
        // renderVector = {
        //     renderParams{
        //         &cubeVAO,
        //         glm::vec3(0.0f,1.0f,0.0f)
        //     }
        // };

        // quadVAO = VAO(quad);
        // framebuffer.completeGenerate(state.width,state.height);


        // Flags
        // objectShaderNeedsToBeChanged = false;
        // objectShaderNeedsToBeChanged = false;
        antialiasing = true;

        // Commands
        sceneCommands = {
            // new KeyCommand(GLFW_KEY_C, [this]() -> void { postprocShaderNeedsToBeChanged = true; }, TOGGLE),
            new KeyCommand(GLFW_KEY_X, [this]() -> void { objectShaderNeedsToBeChanged = true; }, TOGGLE),
            new KeyCommand(GLFW_KEY_V, [this]() -> void { antialiasing = !antialiasing; }, TOGGLE)
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
        
    }

    // MAIN SCENE SPECIFIC METHOD
    void renderScene() override
    {
        glEnable(GL_DEPTH_TEST);

        // framebuffer.bind();

        glClearColor(0.3f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render objects
        glm::mat4 view = state.cam.lookFront();
        renderObjects(view);

        // framebuffer.unbind();
        
        // Draw the Frame on screen
        // glDisable(GL_DEPTH_TEST);

        // glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        // glClear(GL_COLOR_BUFFER_BIT);

        // auto currentShader = postprocShaders[currentPostprocShader];
        // currentShader->use();

        // quadVAO.bind(); 
        // glBindTexture(GL_TEXTURE_2D, framebuffer.colorAttachment->texture);
        // currentShader->setMat4("model",glm::mat4(1.0f));
        // glDrawArrays(GL_TRIANGLES, 0, 6);
        // quadVAO.unbind();
    }

    private:

    void teardown() override 
    // Teardown no sirve mucho porque opengl cuando termina borra todo automaticamente.
    // A lo sumo serviría si tengo que destruir todo lo del engine en medio de la ejecución.
    {
        cubeVAO.deleteBuffers();
        // framebuffer.deleteBuffers();
    }

    void renderObjects(glm::mat4 &view)
    {
        shaders[currentShader]->use();

        glm::mat4 model(1.0f);
        shaders[currentShader]->setVertexMatrices(view, model, state.projectionMatrix);

        // for (auto& [vao, color] : renderVector)
        // {
        //     vao->bind();
        //     shaders[currentShader]->setVec3("color", color);
        //     glDrawArrays(GL_POINTS, 0, vao->renderVertices);
        // }

        cubeVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, cubeVAO.renderVertices);
    }

    void changeShader() 
    {
        // if (postprocShaderNeedsToBeChanged)
        // {
        //     currentPostprocShader = (currentPostprocShader + 1) % postprocShaders.size();
        //     postprocShaderNeedsToBeChanged = false;
        // }
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