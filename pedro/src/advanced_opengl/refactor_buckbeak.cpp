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

#define OBJECTS 0
#define LIGHT_SOURCE 1

typedef unsigned int uint;

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
struct Engine {

public:
    // Window and Camera
    GLFWwindow *window;
    float width = 1280.0f, height = 720.0f;
    float lastX = width / 2.0;
    float lastY = height / 2.0;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    Camera cam = Camera(glm::vec3(0.0f, 0.0f, 10.0f));

    // Shaders
    Shader renderShader;
    uint currentPostprocShader = 0;
    std::vector<Shader> postprocShaders;

    // Textures
    Texture2D* cubeTexture;
    Texture2D* floorTexture;

    // Buffers
    VAO cubeVAO = VAO(cube);
    VAO planeVAO = VAO(plane);
    VAO quadVAO = VAO(quad);
    Framebuffer framebuffer;

    // Commands
    std::array<KeyCommand*, 12> keys = {
    new KeyCommand(GLFW_KEY_R, [this]() -> void { cam.resetFov(); }, TOGGLE),
    
    new KeyCommand(GLFW_KEY_UP, [this]() -> void { cam.speed += 0.1f; }, NORMAL),

    new KeyCommand(GLFW_KEY_DOWN, [this]() -> void { cam.speed -= 0.1f; }, NORMAL),

    new KeyCommand(GLFW_KEY_W, [this]() -> void { cam.moveFront(); }, NORMAL),
                
    new KeyCommand(GLFW_KEY_S, [this]() -> void { cam.moveBack(); }, NORMAL),

    new KeyCommand(GLFW_KEY_A, [this]() -> void { cam.moveLeft(); }, NORMAL),

    new KeyCommand(GLFW_KEY_D, [this]() -> void { cam.moveRight(); }, NORMAL),

    new KeyCommand(GLFW_KEY_SPACE, [this]() -> void { cam.moveWorldUp(); }, NORMAL),

    new KeyCommand(GLFW_KEY_LEFT_SHIFT, [this]() -> void { cam.moveWorldDown(); }, NORMAL),

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
    bool tabPreviouslyPressed = false;
    bool firstMouse = true;
    bool mouseLocked = true;
    bool shaderNeedsToBeChanged = false;

    Engine() {
        init();
    }

    void update() {
        updateFrames();
        processInput();
        changeShader();
    }

private:

    void init() {
        return;
        // Window
        initWindow();

        // Buffers
        framebuffer.bind();
        framebuffer.attatchColor(width, height);
        framebuffer.attatchRender(width, height);
        framebuffer.checkStatus();
        framebuffer.unbind();

        // Shaders
        renderShader = Shader("shaders/shader01.vs", "shaders/shader01.frag");
        postprocShaders = {
            Shader("shaders/post1.vs", "shaders/postDefault.frag"),
            Shader("shaders/post1.vs", "shaders/postInverse.frag"),
            Shader("shaders/post1.vs", "shaders/postGreyscale.frag"),
            Shader("shaders/post1.vs", "shaders/postKernel1.frag"),
            Shader("shaders/post1.vs", "shaders/postKernel2.frag")
        };
        
        // Textures
        cubeTexture = new Texture2D("../media/container.jpg", JPG);
        floorTexture = new Texture2D("../media/metal.png", PNG);

        renderShader.use();
        renderShader.setInt("material.texture_diffuse1", 0);

        // Parameters
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    }

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
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }
    }

    void updateFrames() {
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

    void changeShader() {
        if (shaderNeedsToBeChanged)
        {
            currentPostprocShader = (currentPostprocShader + 1) % postprocShaders.size();
            shaderNeedsToBeChanged = false;
        }
    }

};

int main()
{
    Engine engine;
    return 0;

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    while(!glfwWindowShouldClose(engine.window))
    {
        engine.update();
        
        engine.framebuffer.bind();
        glEnable(GL_DEPTH_TEST);

        // Render:
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // MATRIXES
        model = glm::mat4(1.0f);
        view = engine.cam.lookFront();
        projection = glm::perspective(glm::radians(engine.cam.fov), engine.width / engine.height, 0.1f, 100.0f);

        engine.renderShader.use();
        engine.renderShader.setVertexMatrices(view, model, projection);

        // cubes
        engine.cubeVAO.bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, engine.cubeTexture->texture); 	
        model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
        engine.renderShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        engine.renderShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        engine.cubeVAO.unbind();

        // floor
        engine.planeVAO.bind();

        glBindTexture(GL_TEXTURE_2D, engine.floorTexture->texture);
        engine.renderShader.setMat4("model", glm::mat4(1.0f));
        model = glm::mat4(1.0f);
        engine.renderShader.setVertexMatrices(view, model, projection);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        engine.planeVAO.unbind();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT);

        engine.postprocShaders[engine.currentPostprocShader].use();

        engine.quadVAO.bind();
        glBindTexture(GL_TEXTURE_2D, engine.framebuffer.colorAttachment->texture);
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

#pragma region 

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
    Engine *engine = (Engine*)glfwGetWindowUserPointer(window);
    glViewport(0, 0, _width, _height);
    engine->width = _width;
    engine->height = _height;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    Engine *engine = (Engine*)glfwGetWindowUserPointer(window);

    if (!engine->mouseLocked) return;

    if (engine->firstMouse)
    {
        engine->lastX = xpos;
        engine->lastY = ypos;
        engine->firstMouse = false;
    }
  
    float xoffset = xpos - engine->lastX;
    float yoffset = engine->lastY - ypos; 
    engine->lastX = xpos;
    engine->lastY = ypos;

    engine->cam.handleMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Engine *engine = (Engine*)glfwGetWindowUserPointer(window);

    engine->cam.handleMouseScroll(yoffset);
}

#pragma endregion 