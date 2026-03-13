#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

typedef unsigned int uint;

enum color {BLACK, RED, GREEN, BLUE};

bool setupShadersAndBuffers(uint *shaderProgram, uint *VAO);
GLFWwindow* windowAndContext();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, color *current);
void handleColor(float *r_p, float *g_p, float *b_p, color current_color);

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\n\0";


int main()
{
    GLFWwindow *window = windowAndContext();
    if (window == NULL) return -1;
    
    uint shaderProgram, VAO;

    if (!setupShadersAndBuffers(&shaderProgram, &VAO)) return -1;
    
    color current_color = BLACK;
    float r = 0., g = 0., b = 0.;

    while(!glfwWindowShouldClose(window))
    {
        processInput(window, &current_color);

        // Render:
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        handleColor(&r, &g, &b, current_color);

        uint location = glGetUniformLocation(shaderProgram, "ourColor");
        glUniform4f(location, r, g, b, 1.0f);

        glBindVertexArray(VAO);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);

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

bool setupShadersAndBuffers(uint *shaderProgram, uint *VAO) {

    // Shaders:
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }

    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        return 0;
    }

    // link shaders
    *shaderProgram = glCreateProgram();
    glAttachShader(*shaderProgram, vertexShader);
    glAttachShader(*shaderProgram, fragmentShader);
    glLinkProgram(*shaderProgram);

    // check for linking errors
    glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(*shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    }; 

    unsigned int VBO;
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(*VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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

