#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "stb_image.h"
#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// For writing text on the screen
#include <FTGL/ftgl.h>

#include "cube.cpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, float *fov, float *ratio)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        *fov += 0.1;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        *fov -= 0.1;
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        *ratio += 0.1;
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        *ratio -= 0.1;
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);  

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    float fov = 45.0;
    float ratio = 800.0 / 600.0;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);  

    // Create a pixmap font from a TrueType file.
    FTGLPixmapFont font("../media/Roboto-Regular.ttf");
    // If something went wrong, bail out.
    if(font.Error()) {
        std::cout << "Error opening font: " << font.Error() << std::endl;
        return -1;
    }

    // Render loop
    while(!glfwWindowShouldClose(window))
    {
        // input
        processInput(window, &fov, &ratio);
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
        // ** Model **
        // Rotate the cube over time
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        // ** View **
        glm::mat4 view = glm::mat4(1.0f);
        // note that we're translating the scene in the reverse direction of where we want to move
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        // ** Projection **
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(fov), ratio, 0.1f, 100.0f);

        ourShader.setMatrix4fv("model", glm::value_ptr(model));
        ourShader.setMatrix4fv("view", glm::value_ptr(view));
        ourShader.setMatrix4fv("projection", glm::value_ptr(projection));

        // Select the object to draw
        glBindVertexArray(VAO);
        // Draw all the triangles
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Set the font size and render a small text.
        glColor4f(0.0f,0.5f,0.5f,0.5f);
        font.FaceSize(72);
        font.Render("Hello World!");

        // check and call events and swap buffers
        glfwPollEvents();         
        glfwSwapBuffers(window);
    }

    // Clean up
    glfwTerminate();
    return 0;
}

