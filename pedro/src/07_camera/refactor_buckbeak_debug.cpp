#include <iostream>
#include <string>
#include <array>

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

struct Engine {

};

bool setupBuffers(uint *VAO);
GLFWwindow* windowAndContext();
void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void setLights(Shader &shader);

float width = 1280.0f, height = 720.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool tabPreviouslyPressed = false;

bool firstMouse = true;
bool mouseLocked = true;
bool shaderNeedsToBeChanged = false;
float lastX = width / 2.0, lastY = height / 2.0;

Camera cam(glm::vec3(0.0f, 0.0f, 10.0f));

std::array<KeyCommand*, 12> keys = {
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

    new KeyWindowCommand(GLFW_KEY_ESCAPE, [](GLFWwindow *window) -> void { glfwSetWindowShouldClose(window, true); }, TOGGLE),

    new KeyCommand(GLFW_KEY_C, []() -> void { shaderNeedsToBeChanged = true; }, TOGGLE)
};

int main()
{
    GLFWwindow *window = windowAndContext();
    if (window == NULL) return -1;

    glEnable(GL_DEPTH_TEST);
    
    Shader shader("shaders/shader02.vs", "shaders/shader01.frag");
    VAO myVAO(cube);
    
    // uint VAO;
    // if (!setupBuffers(&VAO)) return -1;

    stbi_set_flip_vertically_on_load(true);

    Texture2D tex1("../media/container.jpg", JPG), tex2("../media/awesomeface.png", PNG);
    
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

    shader.use();

    shader.setInt("tex1", 0);
    shader.setInt("tex2", 1);//


    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        // processInput(window);

        // printf("DEBUG >> DELTA TIME (in ms): %f\n", deltaTime*1000);

        // Render:
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        tex1.activate(0);
        tex2.activate(1);

        glm::mat4 view = cam.lookFront();

        shader.setMat4("view", view);

        // PROJECTION MATRIX
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(cam.fov), width / height, 0.1f, 100.0f);

        shader.setMat4("projection", projection);

        myVAO.bind();

        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i, extraAngle = 1.0;

            if (i % 3 == 0) extraAngle = (float)glfwGetTime();
            
            model = glm::rotate(model, extraAngle * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", model);


            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}

#pragma region 

bool setupBuffers(uint *VAO) {

    uint VBO;
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, &VBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, cube.size() * sizeof(float), cube.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    return 1;
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