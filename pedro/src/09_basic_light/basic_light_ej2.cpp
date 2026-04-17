#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "shader.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define OBJECTS 0
#define LIGHT_SOURCE 1

typedef unsigned int uint;

bool setupBuffers(uint *VAO);
GLFWwindow* windowAndContext();
void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void processInput(GLFWwindow *window, Camera &cam);

float width = 800.0f, height = 600.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool tabPreviouslyPressed = false;

bool firstMouse = true;
bool mouseLocked = true;
float lastX = 800.f / 2.0, lastY = 400.f / 2.0;

bool rotateLight = true;

glm::vec3 lightPos(1.2f, 1.0f, 5.0f);

Camera cam(glm::vec3(0.0f, 0.0f, 5.0f));

int main()
{
    GLFWwindow *window = windowAndContext();
    if (window == NULL) return -1;

    glEnable(GL_DEPTH_TEST);
    
    uint VAO[2];

    if (!setupBuffers(VAO)) return -1;

    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 2.0f,  5.0f, -15.0f), 
        glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f),  
        glm::vec3( 1.5f,  2.0f, -2.5f), 
        glm::vec3( 1.5f,  0.2f, -1.5f), 
        glm::vec3(-1.3f,  1.0f, -1.5f)  
    };

    glm::vec3 cubeColors[] = {
        glm::vec3(0.73f, 0.15f, 0.92f),
        glm::vec3(0.04f, 0.88f, 0.33f),
        glm::vec3(0.56f, 0.21f, 0.77f),
        glm::vec3(0.91f, 0.64f, 0.10f),
        glm::vec3(0.38f, 0.47f, 0.59f),
        glm::vec3(0.12f, 0.99f, 0.25f),
        glm::vec3(0.67f, 0.08f, 0.41f),
        glm::vec3(0.29f, 0.53f, 0.84f),
        glm::vec3(0.75f, 0.36f, 0.02f),
        glm::vec3(0.48f, 0.70f, 0.61f)
    };

    // Setup Shaders
    Shader lightingShader("shaders/lightShader02.vs", "shaders/lightShader02.frag");
    Shader lightSourceShader("shaders/sourceShader01.vs", "shaders/sourceShader01.frag");

    // Setup Textures
    // Texture2D texture("...", JPG/PNG);
    // shader.Use();
    // shader.setInt("texture", 0);

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), (float)glm::radians(1.5), glm::vec3(0.0f,1.0f,0.0f));

    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, cam);
        // processInput(window);

        // printf("DEBUG >> DELTA TIME (in ms): %f\n", deltaTime*1000);

        // Render:
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // VIEW MATRIX
        glm::mat4 view = cam.lookFront();

        // PROJECTION MATRIX
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(cam.fov), width / height, 0.1f, 100.0f);

        // MODEL MATRIX
        glm::mat4 model(1.0f);
        
        // Render Light Source
        glBindVertexArray(VAO[LIGHT_SOURCE]);

        lightSourceShader.use();
        
            // Vertex Uniform
        if (rotateLight)
            lightPos = glm::vec3(rotation * glm::vec4(lightPos, 1.0));

        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));

        lightSourceShader.setVertexMatrices(view, model, projection);
        lightingShader.setVec3("lightPos",  lightPos);

            // Frag Uniform
        // glm::vec3 lightColor = glm::abs(glm::vec3(sin(currentFrame), cos(currentFrame/ 3.0f), sin(currentFrame * 2.0f)));
        glm::vec3 lightColor(1.0f);
        lightSourceShader.setVec3("lightColor", lightColor);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render Objects
        glBindVertexArray(VAO[OBJECTS]);

        lightingShader.use();
            // Compute Model and Normal Matrices
        model = glm::mat4(1.0f);
        
        // glm::mat3 normalMatrix(1.0f);
        // normalMatrix = glm::transpose(glm::inverse((glm::mat3)model)));
        
        // printf("DEBUG >> lightPos1: (%.2f, %.2f, %.2f)\n", lightPos.x, lightPos.y, lightPos.z);

            // Vertex Uniforms
        lightingShader.setVertexMatrices(view, model, projection);
        // lightingShader.setMat3("normalMatrix", normalMatrix);

            // Frag Uniforms

        // lightingShader.setVec3("lightColor",  glm::vec3(1.0f, 1.0f, 1.0f));
        lightingShader.setVec3("lightColor",  lightColor);
        lightingShader.setVec3("viewPos", cam.position);

        for(unsigned int i = 0; i < 10; i++)
        {
            lightingShader.setVec3("objectColor", glm::vec3(0.8f, 0.8f, 0.8f));
            // lightingShader.setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
            // lightingShader.setVec3("objectColor", cubeColors[i]);
            
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i] + glm::vec3(0.0f, 0.0f, 5.0f));
            float angle = 20.0f * i, extraAngle = 1.0;

            if (i % 3 == 0) extraAngle = (float)glfwGetTime();
            
            model = glm::rotate(model, extraAngle * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glDrawArrays(GL_TRIANGLES, 0, 36);


        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}

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

bool setupBuffers(uint *VAO) {

    float vertices[] = {
    // Vertex Pos       | Normal Vector
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
    0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

    uint VBO;

    // VAO[1] for boxes and VAO[2] for Light Source
    glGenVertexArrays(2, VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO[OBJECTS]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Bind to setup light source
    glBindVertexArray(VAO[LIGHT_SOURCE]);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    return 1;
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

