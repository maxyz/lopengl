#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

enum color {BLACK, RED, GREEN, BLUE};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, color* current_color);
void handleColor(float *r, float *g, float *b, color current_color);

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
    float r = 0., g = 0., b = 0.;

    while(!glfwWindowShouldClose(window))
    {
        processInput(window, &current_color);

        // Render:
        handleColor(&r, &g, &b,current_color);
        
        glClearColor(r,g,b,1.0f);
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

    if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
        *current_color = BLACK;
        return;
    }
}

void handleColor(float *r_p, float *g_p, float *b_p, color current_color) {
    float delta = 0.01;
    float r = *r_p, g = *g_p, b = *b_p;
    switch (current_color)
        {
        case BLACK:
            r = r - delta > 0.f ? r - delta : 0.f;
            g = g - delta > 0.f ? g - delta : 0.f;
            b = b - delta > 0.f ? b - delta : 0.f;
            break;

        case RED:
            r = r + delta < 1.f ? r + delta : 1.f;
            g = g - delta > 0.f ? g - delta : 0.f;
            b = b - delta > 0.f ? b - delta : 0.f;
            break;

        case GREEN:
            r = r - delta > 0.f ? r - delta : 0.f;
            g = g - delta < 1.f ? g + delta : 1.f;
            b = b - delta > 0.f ? b - delta : 0.f;
            break;

        case BLUE:
            r = r - delta > 0.f ? r - delta : 0.f;
            g = g - delta > 0.f ? g - delta : 0.f;
            b = b - delta < 1.f ? b + delta : 1.f;
            break;
        }
    
    *r_p = r;
    *g_p = g;
    *b_p = b;
}