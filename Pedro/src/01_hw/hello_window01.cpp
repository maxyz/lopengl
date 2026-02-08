#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

enum color {BLACK, RED, GREEN, BLUE};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, color* current_color);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow* window = glfwCreateWindow(800, 600, "CrazyWindowAction", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 600);
    
    color current_color = BLACK;

    while(!glfwWindowShouldClose(window))
    {
        processInput(window, &current_color);

        // Render:
        switch (current_color)
        {
        case BLACK:
            glClearColor(0.f,0.f,0.f,1.0f);
            break;
        case RED:
            glClearColor(1.f,0.f,0.f,1.0f);
            break;
        case GREEN:
            glClearColor(0.f,1.f,0.f,1.0f);
            break;
        case BLUE:
            glClearColor(0.f,0.f,1.f,1.0f);
            break;
        }
        
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, color* current_color)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }

    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        *current_color = RED;
        return;
    }

    if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        *current_color = GREEN;
        return;
    }

    if(glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        *current_color = BLUE;
        return;
    }
        
}