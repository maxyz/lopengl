#include <iostream>
#include <string>
#include <array>

#include "shader.h"
#include "model.h"
#include "camera.h"
#include "key_command.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define OBJECTS 0
#define LIGHT_SOURCE 1

typedef unsigned int uint;

GLFWwindow* windowAndContext();
void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void setLights(Shader &shader);

float width = 800.0f, height = 600.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool tabPreviouslyPressed = false;

bool firstMouse = true;
bool mouseLocked = true;
float lastX = 800.f / 2.0, lastY = 400.f / 2.0;

Camera cam(glm::vec3(0.0f, 0.0f, 10.0f));

std::array<KeyCommand*, 11> keys = {
    new KeyCommand(GLFW_KEY_R, []() -> void { cam.resetFov(); }, TOGGLE),
    
    new KeyCommand(GLFW_KEY_UP, []() -> void { cam.speed += 0.1f; }, NORMAL),

    new KeyCommand(GLFW_KEY_DOWN, []() -> void { cam.speed -= 0.1f; }, NORMAL),

    new KeyCommand(GLFW_KEY_W, []() -> void { cam.moveFront(); }, NORMAL),
                
    new KeyCommand(GLFW_KEY_S, []() -> void { cam.moveBack(); }, NORMAL),

    new KeyCommand(GLFW_KEY_A, []() -> void { cam.moveLeft(); }, NORMAL),

    new KeyCommand(GLFW_KEY_D, []() -> void { cam.moveRight(); }, NORMAL),

    new KeyCommand(GLFW_KEY_SPACE, []() -> void { cam.moveWorldUp(); }, NORMAL),

    new KeyCommand(GLFW_KEY_LEFT_SHIFT, []() -> void { cam.moveWorldDown(); }, NORMAL),

    new KeyWindowCommand(GLFW_KEY_TAB, 
        [](GLFWwindow *window) -> void { 
            if (mouseLocked) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                mouseLocked = false;
                firstMouse = true;
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                mouseLocked = true;
            }
        }, TOGGLE),

    new KeyWindowCommand(GLFW_KEY_ESCAPE, [](GLFWwindow *window) -> void { glfwSetWindowShouldClose(window, true); }, TOGGLE)
};

int main()
{
    GLFWwindow *window = windowAndContext();
    if (window == NULL) return -1;

    glEnable(GL_DEPTH_TEST);
    // glDepthMask(GL_FALSE); Si por alguna razón necesitamos que el buffer sea read only
    // glDepthFunc(GL_*COMPARISON CONSTANT*); // Para cambiar la función de testing 
     

    // Setup Shaders
    Shader lightingShader("shaders/lightShader01.vs", "shaders/lightShader01.frag");

    // Texture2D emissionMap("../media/matrix.jpg", JPG);
    lightingShader.use();

    std::string path = "../models/backpack/backpack.obj";
    Model objects(path);

    // glm::mat4 source_rotation = glm::rotate(glm::mat4(1.0f), (float)glm::radians(1.5), glm::vec3(0.0f,1.0f,0.0f));

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Render:
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // MATRIXES
        model = glm::mat4(1.0f);
        view = cam.lookFront();
        projection = glm::perspective(glm::radians(cam.fov), width / height, 0.1f, 100.0f);

        lightingShader.setVertexMatrices(view, model, projection);

        // #### RENDER OBJECTS ####
        objects.draw(lightingShader);

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}

#pragma region 

GLFWwindow* windowAndContext() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow *window = glfwCreateWindow(width, height, "CrazyWindowAction", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    return window;
}

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
    glViewport(0, 0, _width, _height);
    width = _width;
    height = _height;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cam.handleMouseScroll(yoffset);
}

void processInput(GLFWwindow *window)
{
    cam.deltaSpeed = cam.speed * deltaTime;
    for (auto key : keys)
    {
        key->detect(window);
    }
}


#pragma endregion 