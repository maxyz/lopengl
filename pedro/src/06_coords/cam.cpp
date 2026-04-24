#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef unsigned int uint;

bool setupBuffers(uint *VAO);
GLFWwindow* windowAndContext();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void processInput(GLFWwindow *window, float &fov, glm::vec3 &cameraPos);

int main()
{
    GLFWwindow *window = windowAndContext();
    if (window == NULL) return -1;

    glEnable(GL_DEPTH_TEST);
    
    Shader shader("shaders/shader02.vs", "shaders/shader01.frag");
    uint VAO;

    if (!setupBuffers(&VAO)) return -1;

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
    shader.setInt("tex2", 1);

    float fov = 45.f;
    glm::vec3 cameraPos(0.0f, 0.0f, -2.0f);

    while(!glfwWindowShouldClose(window))
    {
        // processInput(window, fov, cameraPos);
        processInput(window, fov, cameraPos);

        printf("DEBUG: CAM POSITION %f %f %f\n", cameraPos.x, cameraPos.y, cameraPos.z);

        // Render:
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        tex1.activate(0);
        tex2.activate(1);

        // // VIEW MATRIX
        // const float radius = 10.0f;
        // float camX = sin(glfwGetTime()) * radius;
        // float camZ = cos(glfwGetTime()) * radius;
        // glm::mat4 view;
        // view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));  

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, cameraPos);

        shader.setMat4("view", view);

        // PROJECTION MATRIX
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        shader.setMat4("projection", projection);

        glBindVertexArray(VAO);

        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i, extraAngle = 1.0;

            if (i % 3 == 0) extraAngle = (float)glfwGetTime();
            
            model = glm::rotate(model, extraAngle * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", projection);


            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

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
  
    GLFWwindow *window = glfwCreateWindow(800, 600, "CrazyWindowAction", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    return window;
}

bool setupBuffers(uint *VAO) {

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };  

    uint VBO;
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, &VBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, float &fov, glm::vec3 &cameraPos)
{
    
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }

    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        fov += 1.0f;
    
    }

    if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        fov -= 1.0f;
        
    }

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += glm::vec3(0.0f, 0.0f, 0.1f);
        
    }

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= glm::vec3(0.0f, 0.0f, 0.1f);
        
    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos += glm::vec3(0.1f, 0.0f, 0.0f);
        
    }

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos -= glm::vec3(0.1f, 0.0f, 0.0f);
        
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cameraPos -= glm::vec3(0.0f, 0.1f, 0.0f);
        
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraPos += glm::vec3(0.0f, 0.1f, 0.0f);
        
    }
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }        
}