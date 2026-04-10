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

void processInput(GLFWwindow *window, float deltaTime, glm::vec3 *cameraPos, glm::vec3 *cameraFront, glm::vec3 *cameraUp)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 2.5f * deltaTime;
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        *cameraPos += cameraSpeed * *cameraUp;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        *cameraPos -= cameraSpeed * *cameraUp;
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        *cameraPos += glm::normalize(glm::cross(*cameraFront, *cameraUp)) * cameraSpeed;;
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        *cameraPos -= glm::normalize(glm::cross(*cameraFront, *cameraUp)) * cameraSpeed;;
    if(glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        *cameraPos += cameraSpeed * *cameraFront;
    if(glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        *cameraPos -= cameraSpeed * *cameraFront;
/*    if(glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
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
        *rotz += 1;*/
}

// Mouse input requires a number of global variables :-/
float lastX = 400, lastY = 300;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;

// Handle mouse input
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    if (firstMouse) {
        firstMouse = false;
        return;
    }

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset; 

    // Avoid weird jumps looking up or down
    if(pitch > 89.0f)
      pitch =  89.0f;
    if(pitch < -89.0f)
      pitch = -89.0f;

}

float fov = 45.0f;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 60.0f)
        fov = 60.0f;
}


glm::mat4 my_lookAt(glm::vec3 P, glm::vec3 target, glm::vec3 up)
{
    glm::vec3 D = glm::normalize(P - target);
    glm::vec3 R = glm::normalize(glm::cross(up, D));
    glm::vec3 U = glm::cross(D, R);

    glm::mat4 M = glm::mat4(1.0);
    M[0] =  glm::vec4(R.x, U.x, D.x, 0.0);
    M[1] =  glm::vec4(R.y, U.y, D.y, 0.0);
    M[2] =  glm::vec4(R.z, U.z, D.z, 0.0);
    glm::mat4 T = glm::mat4(1.0);
    T[3] = glm::vec4(-P, 1.0);

    return M * T;
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

    float ratio = 800.0 / 600.0;

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);  

    // Capture the mouse and call the callback function
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Render loop
    while(!glfwWindowShouldClose(window))
    {
        // Calculate delta
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window, deltaTime, &cameraPos, &cameraFront, &cameraUp);
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
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        glm::mat4 view;
        view = my_lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        //
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
        writer.write( std::format("Pos ({:.2f}, {:.2f}, {:.2f})", cameraPos.x, cameraPos.y, cameraPos.z) );
        // Position the text in the screen
        textModel = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 510.0f, 0.0f));
        fontShader.setMatrix4fv("model", glm::value_ptr(textModel));
        writer.write( std::format("Front ({:.2f}, {:.2f}, {:.2f})", cameraFront.x, cameraFront.y, cameraFront.z) );

        // check and call events and swap buffers
        glfwPollEvents();         
        glfwSwapBuffers(window);
    }

    // Clean up
    glfwTerminate();
    return 0;
}

