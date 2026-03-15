#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "stb_image.h"
#include "shader.h"
#include "texture.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, float *scale, float *mode1, float *mode2)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        *scale += 0.1;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        *scale -= 0.1;
    if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        *mode1 = 0;
    if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        *mode1 = 1;
    if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        *mode1 = 2;
    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        *mode1 = 3;
    if(glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        *mode2 = 0;
    if(glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        *mode2 = 1;
    if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        *mode2 = 2;
    if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        *mode2 = 3;
}

GLenum getMode(float mode)
{
    switch (int(mode) % 4) {
        case 0:
            return GL_REPEAT;
        case 1:
            return GL_MIRRORED_REPEAT;
        case 2:
            return GL_CLAMP_TO_EDGE;
        case 3:
            return GL_CLAMP_TO_BORDER;
    }
    return GL_REPEAT;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  
    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Textures", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    int version = gladLoadGL(glfwGetProcAddress);
    if (!version)
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Initial size and call back for resizing
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    Texture::flip_vertically();

    Texture texture1 = Texture("media/container.jpg", GL_RGB);
    Texture texture2 = Texture("media/awesomeface.png", GL_RGBA);

    // Read shaders
    Shader ourShader("shaders/texture-shader.vs", "shaders/ej2.fs");

    // Rectangle with texture
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
    };
    // Create an Element Buffer Object
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    // Create a buffer object called VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    // Generate a Vertex array
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);  

    // ..:: Initialization code (done once (unless your object frequently changes)) :: ..
    // 1. bind Vertex Array Object
    glBindVertexArray(VAO);
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3. copy our index array in a element buffer for OpenGL to use
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // 4. then set our vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);  

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    float scale = 2.0;
    float mode1 = 2.0;
    float mode2 = 0.0;

    // Render loop
    while(!glfwWindowShouldClose(window))
    {
        // input
        processInput(window, &scale, &mode1, &mode2);
        // rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ourShader.use();
        ourShader.setFloat("scale", scale);

        // Bind the texture to draw
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1.ID);
	texture1.set_wrap(getMode(mode1));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2.ID);
        texture2.set_wrap(getMode(mode2));
        // Select the object to draw
        glBindVertexArray(VAO);
        // Draw triangle
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // check and call events and swap buffers
        glfwPollEvents();         
        glfwSwapBuffers(window);
    }

    // Clean up
    glfwTerminate();
    return 0;
}

