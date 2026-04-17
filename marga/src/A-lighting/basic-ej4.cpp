#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <format>
#include <cmath>
#include "stb_image.h"
#include "shader.h"
#include "write_text.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "lighting_vertices.cpp"

const int INITIAL_WIDTH = 800;
const int INITIAL_HEIGHT = 600;

float width = (float) INITIAL_WIDTH, height = (float) INITIAL_HEIGHT;

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
    glViewport(0, 0, width, height);
    width = (float) _width;
    height = (float) _height;

}

// Mouse input requires a number of global variables :-/
Camera camera = Camera(glm::vec3(0.0f, 0.0f, 5.0f));
double lastX = 400, lastY = 300;
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
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow *window, float deltaTime, Camera &camera)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 2.5f * deltaTime;
    Camera_Movement direction = NONE;
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        direction = UP;
    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        direction = DOWN;
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        direction = RIGHT;
    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        direction = LEFT;
    if(glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
        direction = FORWARD;
    if(glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
        direction = BACKWARD;

    if (direction != NONE)
        camera.ProcessKeyboard(direction, deltaTime);

    // Right click releases the mouse
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPosCallback(window, NULL);
    }
    // Left click captures the mouse again
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &lastX, &lastY);
        firstMouse = true;
        glfwSetCursorPosCallback(window, mouse_callback);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow* window = glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "Lighting", NULL, NULL);
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
    glViewport(0, 0, INITIAL_WIDTH, INITIAL_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 

    // Read shaders
    Shader ourShader("shaders/ej4-vertex.glsl", "shaders/ej4-fragment.glsl");
    Shader sourceShader("shaders/vertex.glsl", "shaders/source-frag.glsl");

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
    // 3. then set our vertex attributes pointers (the vertices are in lighting_vertices.cpp)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ..:: Create a light source object ::..
    // It has its own VAO.
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // we only need to bind to the VBO, the container's VBO's data already contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // set the vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Position of the light
    glm::vec3 lightPos(1.2f, 0.0f, 0.0f);
    ourShader.use();
    ourShader.setVec3f("lightPos", glm::value_ptr(lightPos));

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

    TextWriter writer = TextWriter("../media/Roboto-Regular.ttf");
    Shader fontShader("shaders/font_vertex.glsl", "shaders/font_fragment.glsl");

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
        processInput(window, deltaTime, camera);
        // rendering commands here
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        ourShader.setVec3f("objectColor", (float[]) {1.0f, 0.5f, 0.31f});
        ourShader.setVec3f("lightColor",  (float[]) {1.0f, 1.0f, 1.0f});
        ourShader.setVec3f("viewPos", glm::value_ptr(camera.Position));

        // Coordinate matrixes
        // ** View **
        glm::mat4 view = camera.GetViewMatrix();
        // ** Projection **
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.Zoom), width/height, 0.1f, 100.0f);

        ourShader.setMatrix4fv("view", glm::value_ptr(view));
        ourShader.setMatrix4fv("projection", glm::value_ptr(projection));

        // Draw each of the 10 cubes, with a different model matrix
        glBindVertexArray(VAO);
        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            //float angle = (float)glfwGetTime() * 45.0f;
            float angle = i * 45.0f;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMatrix4fv("model", glm::value_ptr(model));

            float objColor[] = {1.0f-i*0.1f, 0.5f+i*0.03f, 0.31f+i*0.05f};
            ourShader.setVec3f("objectColor", objColor);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Draw the light on the screen
        sourceShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        sourceShader.setMatrix4fv("view", glm::value_ptr(view));
        sourceShader.setMatrix4fv("projection", glm::value_ptr(projection));
        sourceShader.setMatrix4fv("model", glm::value_ptr(model));
        // draw the light cube object
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // -*- Write text on screen -*-
        fontShader.use();
        fontShader.setVec4f("textColor", glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))); // white
        // Orthographic projection: pixel coords, origin bottom-left
        glm::mat4 textProj = glm::ortho(0.0f, width, 0.0f, height);
        fontShader.setMatrix4fv("projection", glm::value_ptr(textProj));
        // Position the text in the screen
        glm::mat4 textModel = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, height-30.0, 0.0f));
        fontShader.setMatrix4fv("model", glm::value_ptr(textModel));
        writer.write( std::format("Pos ({:.2f}, {:.2f}, {:.2f})", camera.Position.x, camera.Position.y, camera.Position.z) );

        // Position the text in the screen
        textModel = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, height-70.0, 0.0f));
        fontShader.setMatrix4fv("model", glm::value_ptr(textModel));
        writer.write( std::format("Front ({:.2f}, {:.2f}, {:.2f})", camera.Front.x, camera.Front.y, camera.Front.z) );

        // check and call events and swap buffers
        glfwPollEvents();         
        glfwSwapBuffers(window);
    }

    // Clean up
    glfwTerminate();
    return 0;
}

