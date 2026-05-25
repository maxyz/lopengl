#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#include "shader.h"
#include "model.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define OBJECTS 0
#define LIGHT_SOURCE 1

typedef unsigned int uint;

struct Light {
    glm::vec3 direction;
    glm::vec3 color;   
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float ambient_intensity;
    float diff_intensity;

    bool lightOn;

    void changeColor(const glm::vec3 newColor) {
        color = newColor;
        diffuse = diff_intensity * color;
        ambient = ambient_intensity * diffuse;
        specular = color;
    }
};

Light light1 = {
    glm::vec3(-0.2f, -1.0f, -0.3f),     // Direction
    glm::vec3(1.0f),                    // Color
    glm::vec3(0.2f),                    // Ambient
    glm::vec3(1.0f),                    // Diffuse
    glm::vec3(1.0f),                    // Specular
    0.2f,
    0.8f,
    true
};  

GLFWwindow* windowAndContext();
void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void processInput(GLFWwindow *window, Camera &cam);
void setLight(Shader &shader, const Light &light);

float width = 800.0f, height = 600.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool tabPreviouslyPressed = false;

bool firstMouse = true;
bool mouseLocked = true;
float lastX = 800.f / 2.0, lastY = 400.f / 2.0;

bool rotateLight = true;
bool coolLightMode = 0;

Camera cam(glm::vec3(0.0f, 0.0f, 10.0f));

int main()
{
    GLFWwindow *window = windowAndContext();
    if (window == NULL) return -1;

    glEnable(GL_DEPTH_TEST);

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

        processInput(window, cam);

        // Render:
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // MATRIXES
        model = glm::mat4(1.0f);
        view = cam.lookFront();
        projection = glm::perspective(glm::radians(cam.fov), width / height, 0.1f, 100.0f);

        setLight(lightingShader, light1);
        lightingShader.setVec3("viewPos", cam.position);

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

void processInput(GLFWwindow *window, Camera &cam)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }

    // SETTINGS

    if(!tabPreviouslyPressed && glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (mouseLocked) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mouseLocked = false;
            firstMouse = true;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            mouseLocked = true;
        }
        
    }
    tabPreviouslyPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;

    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        cam.resetFov();
    }

    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        rotateLight = !rotateLight;
    }

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cam.speed += 0.1f;
    }

    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        cam.speed -= 0.1f;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        coolLightMode = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        coolLightMode = 1;
    }
    
    // MOVEMENT
    cam.deltaSpeed = cam.speed * deltaTime;
    
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.moveFront();
    }

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.moveBack();   
    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.moveLeft();
    }

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.moveRight();
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cam.moveWorldUp();
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cam.moveWorldDown();
    }
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }        
}

void setLight(Shader &shader, const Light &light) {
    shader.setVec3("light.direction", light.direction);
    shader.setVec3("light.ambient", light.ambient);
    shader.setVec3("light.diffuse", light.diffuse);
    shader.setVec3("light.specular", light.specular);
}

#pragma endregion 