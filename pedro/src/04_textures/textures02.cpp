#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "shader.h"
#include "texture.h"

typedef unsigned int uint;

bool setupBuffers(uint *VAO);
GLFWwindow* windowAndContext();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

int main()
{
    GLFWwindow *window = windowAndContext();
    if (window == NULL) return -1;
    
    Shader shader("shaders/shader01.vs", "shaders/shader02.frag");
    uint VAO;

    if (!setupBuffers(&VAO)) return -1;

    stbi_set_flip_vertically_on_load(true);

    Texture2D tex1("../media/container.jpg", JPG), tex2("../media/awesomeface.png", PNG);
    
    shader.use();

    shader.setInt("tex1", 0);
    shader.setInt("tex2", 1);

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Render:
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();

        tex1.activate(0);
        tex2.activate(1);

        glBindVertexArray(VAO);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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
        // positions          // colors           // texture coords
        0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };

    uint indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };  

    uint VBO, EBO;
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Datos de posición
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Datos de color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Datos de textura
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

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

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }        
}

