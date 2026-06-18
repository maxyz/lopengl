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

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebufferSizeCallback(GLFWwindow* window, int _width, int _height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

struct Engine {

public:
    // Window and Camera
    GLFWwindow *window;
    float width;
    float height;
    float lastX;
    float lastY;
    float deltaTime;
    float lastFrame;
    Camera cam;
    glm::mat4 projectionMatrix;

    // Shaders
    Shader renderShader;
    std::vector<Shader> postprocShaders;
    uint currentPostprocShader;

    // Textures
    Texture2D cubeTexture;
    Texture2D floorTexture;

    // Buffers
    struct renderParams {
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

    // Commands
    std::array<KeyCommand*, 12> keys;

    // Flags
    bool tabPreviouslyPressed;
    bool firstMouse;
    bool mouseLocked;
    bool shaderNeedsToBeChanged;

    Engine() 
    {
        // Window and Camera
        width = 1280.0f;
        height = 720.0f;
        lastX = width / 2.0;
        lastY = height / 2.0;
        deltaTime = 0.0f;
        lastFrame = 0.0f;
        cam.move(glm::vec3(0.0f, 0.0f, 10.0f));
        initWindow();
        updateProjection();
        
        // Shaders
        renderShader = Shader("shaders/shader01.vs", "shaders/shader01.frag");
        postprocShaders = {
            Shader("shaders/post2.vs", "shaders/postDefault.frag"),
            Shader("shaders/post2.vs", "shaders/postInverse.frag"),
            Shader("shaders/post2.vs", "shaders/postGreyscale.frag"),
            Shader("shaders/post2.vs", "shaders/postKernel1.frag"),
            Shader("shaders/post2.vs", "shaders/postKernel2.frag")
        };
        currentPostprocShader = 0;

        // Textures
        cubeTexture = Texture2D("../media/container.jpg", JPG);
        floorTexture = Texture2D("../media/metal.png", PNG);
        renderShader.use();
        renderShader.setInt("material.texture_diffuse1", 0);

        // Buffers
        cubeVAO = VAO(cube);
        planeVAO = VAO(plane);
        renderVector = {
            {&cubeVAO, glm::vec3(-1.0f, 0.0f, -1.0f), &cubeTexture},
            {&cubeVAO, glm::vec3(2.0f, 0.0f, 0.0f), &cubeTexture},
            {&planeVAO, glm::vec3(0.0f), &floorTexture}
        };

        quadVAO = VAO(quad);
        frameFrontView.completeGenerate(width,height);
        frameRearView.completeGenerate(width,height);

        // Commands
        keys = {
            new KeyCommand(GLFW_KEY_R, [this]() -> void { this->cam.resetFov(); }, TOGGLE),
            
            new KeyCommand(GLFW_KEY_UP, [this]() -> void { this->cam.speed += 0.1f; }, NORMAL),

            new KeyCommand(GLFW_KEY_DOWN, [this]() -> void { this->cam.speed -= 0.1f; }, NORMAL),

            new KeyCommand(GLFW_KEY_W, [this]() -> void { this->cam.moveFront(); }, NORMAL),
                        
            new KeyCommand(GLFW_KEY_S, [this]() -> void { this->cam.moveBack(); }, NORMAL),

            new KeyCommand(GLFW_KEY_A, [this]() -> void { this->cam.moveLeft(); }, NORMAL),

            new KeyCommand(GLFW_KEY_D, [this]() -> void { this->cam.moveRight(); }, NORMAL),

            new KeyCommand(GLFW_KEY_SPACE, [this]() -> void { this->cam.moveWorldUp(); }, NORMAL),

            new KeyCommand(GLFW_KEY_LEFT_SHIFT, [this]() -> void { this->cam.moveWorldDown(); }, NORMAL),

            new KeyWindowCommand(GLFW_KEY_TAB, 
                [this](GLFWwindow *window) -> void { 
                    if (mouseLocked) {
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        mouseLocked = false;
                        firstMouse = true;
                    } else {
                        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                        mouseLocked = true;
                    }
                }, TOGGLE),

            new KeyWindowCommand(GLFW_KEY_ESCAPE, [this](GLFWwindow *window) -> void { glfwSetWindowShouldClose(window, true); }, TOGGLE),

            new KeyCommand(GLFW_KEY_C, [this]() -> void { shaderNeedsToBeChanged = true; }, TOGGLE)
        };

        // Flags
        tabPreviouslyPressed = false;
        firstMouse = true;
        mouseLocked = true;
        shaderNeedsToBeChanged = false;

        // Parameters
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

    void update() 
    {
        updateFrames();
        processInput();
        changeShader();
        updateProjection();
    }

    void renderScene(glm::mat4 &view)
    {
        renderShader.use();
        glm::mat4 model(1.0f);
        renderShader.setVertexMatrices(view, model, projectionMatrix);
        
        glActiveTexture(GL_TEXTURE0);
        for (auto& [vao, translate, texture] : renderVector)
        {
            vao->bind();
            glBindTexture(GL_TEXTURE_2D, texture->texture);
            model = glm::translate(glm::mat4(1.0f), translate);
            renderShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, vao->renderVertices);
        }        
    }

    void handleFramebufferSizeCallback(GLFWwindow* window, int _width, int _height)
    {
        glViewport(0, 0, _width, _height);
        width = _width;
        height = _height;
    }

    void handleMouseCallback(GLFWwindow* window, double xpos, double ypos) 
    {
        if (!mouseLocked) return;

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }
    
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; 
        lastX = xpos;
        lastY = ypos;

        cam.handleMouseMovement(xoffset, yoffset);
    }

    void handleScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        cam.handleMouseScroll(yoffset);
    }

private:

    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
        window = glfwCreateWindow(width, height, "CrazyWindowAction", NULL, NULL);

        if (window == NULL)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwSetWindowUserPointer(window,this);
        glfwMakeContextCurrent(window);
        
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetCursorPosCallback(window, mouseCallback);
        glfwSetScrollCallback(window, scrollCallback);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }
    }

    void updateFrames()
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
    }

    void processInput()
    {
        cam.deltaSpeed = cam.speed * deltaTime;
        for (auto key : keys)
        {
            key->detect(window);
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

    void updateProjection() {
        projectionMatrix = glm::perspective(glm::radians(cam.fov), width / height, 0.1f, 100.0f);
    }

};

int main()
{
    Engine engine;

    glm::mat4 view;

    while(!glfwWindowShouldClose(engine.window))
    {
        engine.update();
        
        glEnable(GL_DEPTH_TEST);

        // Render Front View
        engine.frameFrontView.bind();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = engine.cam.lookFront();
        engine.renderScene(view);

        engine.frameFrontView.unbind();

        // Render Rear View
        engine.frameRearView.bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = engine.cam.lookRearView();
        engine.renderScene(view);

        engine.frameRearView.unbind();

        glDisable(GL_DEPTH_TEST);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        auto& currentShader = engine.postprocShaders[engine.currentPostprocShader];
        currentShader.use();

        engine.quadVAO.bind();
        glBindTexture(GL_TEXTURE_2D, engine.frameFrontView.colorAttachment->texture);
        currentShader.setMat4("model",glm::mat4(1.0f));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindTexture(GL_TEXTURE_2D, engine.frameRearView.colorAttachment->texture);
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.8f, 0.0f));
        model = glm::scale(model, glm::vec3(0.2f));
        currentShader.setMat4("model",model);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        engine.quadVAO.unbind();

        glfwSwapBuffers(engine.window);
        glfwPollEvents();    
    }

    engine.cubeVAO.deleteBuffers();
    engine.planeVAO.deleteBuffers();
    engine.quadVAO.deleteBuffers();

    glfwTerminate();
    return 0;
}

void framebufferSizeCallback(GLFWwindow* window, int _width, int _height)
{
    Engine *engine = (Engine*)glfwGetWindowUserPointer(window);
    engine->handleFramebufferSizeCallback(window, _width, _height);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) 
{
    Engine *engine = (Engine*)glfwGetWindowUserPointer(window);
    engine->handleMouseCallback(window,xpos,ypos);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    Engine *engine = (Engine*)glfwGetWindowUserPointer(window);
    engine->handleScrollCallback(window, xoffset, yoffset);
}