#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <format>
#include <cmath>
#include "stb_image.h"
#include "shader.h"
#include "texture.h"
#include "write_text.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cube.cpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, float *camx, float *camy, float *camz, float *rotx, float *roty, float *rotz)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        *camy -= 0.1;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        *camy += 0.1;
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        *camx -= 0.1;
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        *camx += 0.1;
    if(glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        *camz += 0.1;
    if(glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        *camz -= 0.1;
    if(glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
        *rotx -= 1;
    if(glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
        *rotx += 1;
    if(glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
        *roty -= 1;
    if(glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
        *roty += 1;
    if(glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS)
        *rotz -= 1;
    if(glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
        *rotz += 1;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  
    GLFWwindow* window = glfwCreateWindow(800, 600, "Coordinate Systems", NULL, NULL);
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

    Texture texture1 = Texture("../media/container.jpg", GL_RGB);
    Texture texture2 = Texture("../media/awesomeface.png", GL_RGBA);

    // Read shaders
    Shader ourShader("shaders/vertex.glsl", "shaders/fragment.glsl");

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
    // 3. then set our vertex attributes pointers (the vertices are in cube.cpp)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);  

    // We have 10 cubes
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

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    TextWriter writer = TextWriter("../media/Roboto-Regular.ttf");
    Shader fontShader("shaders/font_vertex.glsl", "shaders/font_fragment.glsl");

    float fov = 45.0;
    float ratio = 800.0 / 600.0;
    float camx = 0.0f, camy = 0.0f, camz = -3.0f;
    float rotx = 1.0f, roty = 1.0f, rotz = 1.0f;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);  

    // Render loop
    while(!glfwWindowShouldClose(window))
    {
        // input
        processInput(window, &camx, &camy, &camz, &rotx, &roty, &rotz);
        // rendering commands here
        glClearColor(0.5f, 0.3f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        ourShader.setFloat("alpha", 0.2f);

        // Bind the texture to draw
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1.ID);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2.ID);

        // Coordinate matrixes
        // ** View **
        glm::mat4 view = glm::mat4(1.0f);
        // note that we're translating the scene in the reverse direction of where we want to move
        view = glm::translate(view, glm::vec3(camx, camy, camz));
        view = glm::rotate(view, glm::radians(rotx), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::rotate(view, glm::radians(roty), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(rotz), glm::vec3(0.0f, 0.0f, 1.0f));
        // ** Projection **
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(fov), ratio, 0.1f, 100.0f);

        ourShader.setMatrix4fv("view", glm::value_ptr(view));
        ourShader.setMatrix4fv("projection", glm::value_ptr(projection));


        // Draw each of the 10 cubes, with a different model matrix
        glBindVertexArray(VAO);
        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * (i+1);
            if (i % 3 == 0) {
                angle = (float)glfwGetTime() * 45.0f;
            } 
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMatrix4fv("model", glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // -*- Write text on screen -*-
        fontShader.use();
        fontShader.setVec4f("textColor", glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))); // white
        // Orthographic projection: pixel coords, origin bottom-left
        glm::mat4 textProj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
        fontShader.setMatrix4fv("projection", glm::value_ptr(textProj));
        // Position the text in the screen
        glm::mat4 textModel = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 550.0f, 0.0f));
        fontShader.setMatrix4fv("model", glm::value_ptr(textModel));
        writer.write( std::format("Cam X: {:.2f} - Y: {:.2f} - Z: {:.2f}", camx, camy, camz) );
        // Position the text in the screen
        textModel = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 510.0f, 0.0f));
        fontShader.setMatrix4fv("model", glm::value_ptr(textModel));
        writer.write( std::format("Rot X: {:.2f}, - Y: {:.2f} - Z: {:.2f}", rotx, roty, rotz) );

        // check and call events and swap buffers
        glfwPollEvents();         
        glfwSwapBuffers(window);
    }

    // Clean up
    glfwTerminate();
    return 0;
}

