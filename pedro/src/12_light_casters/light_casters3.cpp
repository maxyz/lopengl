#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "shader.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "material.cpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define OBJECTS 0
#define LIGHT_SOURCE 1

typedef unsigned int uint;

struct Light {
    glm::vec3 color;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    float ambient_intensity;
    float diff_intensity;

    void changeColor(const glm::vec3 newColor) {
        color = newColor;
        diffuse = diff_intensity * color;
        ambient = ambient_intensity * diffuse;
        specular = color;
    }
};

bool setupBuffers(uint *VAO);
GLFWwindow* windowAndContext();
void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void processInput(GLFWwindow *window, Camera &cam);
void setMaterial(Shader &shader, const material::Material &mat);
void setLight(Shader &shader, const Light &light);

float width = 800.0f, height = 600.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool tabPreviouslyPressed = false;

bool firstMouse = true;
bool mouseLocked = true;
float lastX = 800.f / 2.0, lastY = 400.f / 2.0;

bool rotateLight = true;
bool coolLightMode = 0;

Light light1 = {
    glm::vec3(1.0f),                // Color

    glm::vec3(0.2f, 0.2f, 0.2f),    // Ambient
    glm::vec3(0.5f, 0.5f, 0.5f),    // Diffuse
    glm::vec3(1.0f, 1.0f, 1.0f),    // Specular

    1.0f,                           // Constant
    0.045f,                         // Linear
    0.0075f,                        // Quadratic

    0.2f,                           // Ambient Intensity
    0.8f                            // Diffuse Intensity
};

Camera cam(glm::vec3(0.0f, 0.0f, 10.0f));

int main()
{
    GLFWwindow *window = windowAndContext();
    if (window == NULL) return -1;

    glEnable(GL_DEPTH_TEST);
    
    uint VAO[2];

    if (!setupBuffers(VAO)) return -1;

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

    // Setup Shaders
    Shader lightingShader("shaders/lightShader01.vs", "shaders/lightShader03.frag");
    Shader lightSourceShader("shaders/sourceShader01.vs", "shaders/sourceShader01.frag");

    // Setup Textures
    Texture2D containerTexture("../media/container2.png", PNG);
    Texture2D specularMap("../media/container2_specular2.png", PNG);
    Texture2D emissionMap("../media/matrix.jpg", JPG);
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);
    lightingShader.setInt("material.emission", 2);


    glm::mat4 source_rotation = glm::rotate(glm::mat4(1.0f), (float)glm::radians(1.5), glm::vec3(0.0f,1.0f,0.0f));

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    Light *currentLight = &light1;

    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, cam);

        // Render:
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // MATRIXES
        model = glm::mat4(1.0f);
        view = cam.lookFront();
        projection = glm::perspective(glm::radians(cam.fov), width / height, 0.1f, 100.0f);

        // #### LIGHT SOURCE ####
        glBindVertexArray(VAO[LIGHT_SOURCE]);

        lightSourceShader.use();
        
        // VERTEX SHADER
        // if (rotateLight)
        //     currentLight->position = glm::vec3(source_rotation * glm::vec4(currentLight->position, 1.0));

        // model = glm::translate(model, currentLight->position);
        // model = glm::scale(model, glm::vec3(0.2f));

        lightSourceShader.setVertexMatrices(view, model, projection);

        // FRAGMENT SHADER
        if (coolLightMode) {
            currentLight->changeColor(glm::abs(glm::vec3(sin(currentFrame), cos(currentFrame/ 3.0f), sin(currentFrame * 2.0f)))); 
        } else {
            currentLight->changeColor(glm::vec3(1.0f));
        }

        lightSourceShader.setVec3("lightColor", currentLight->color);

        // glDrawArrays(GL_TRIANGLES, 0, 36);

        

        // #### OBJECTS ####
        glBindVertexArray(VAO[OBJECTS]);

        lightingShader.use();

        // material properties
        lightingShader.setFloat("material.shininess", 64.0f);
        containerTexture.activate(0);
        specularMap.activate(1);
        emissionMap.activate(2);


        model = glm::mat4(1.0f);

        // VERTEX SHADER
        lightingShader.setVertexMatrices(view, model, projection);

        // FRAGMENT SHADER
        lightingShader.setVec3("viewPos", cam.position);

        setLight(lightingShader, *currentLight);
        setMaterial(lightingShader, material::cyan_glazed_ceramic);

        for(unsigned int i = 0; i < 10; i++)
        {            
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i] + glm::vec3(0.0f, 0.0f, 5.0f));
            float angle = 20.0f * i, extraAngle = 1.0;

            if (i % 3 == 0) extraAngle = (float)glfwGetTime();
            
            model = glm::rotate(model, extraAngle * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}

#pragma region 

GLFWwindow* windowAndContext() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow *window = glfwCreateWindow(width, height, "CrazyWindowAction", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    return window;
}

bool setupBuffers(uint *VAO) {

    float vertices[] = {
    // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    uint VBO;

    // VAO[1] for boxes and VAO[2] for Light Source
    glGenVertexArrays(2, VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO[OBJECTS]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Bind to setup light source
    glBindVertexArray(VAO[LIGHT_SOURCE]);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    return 1;
}

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
    glViewport(0, 0, _width, _height);
    width = _width;
    height = _height;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (!mouseLocked) return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
  
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos;
    lastY = ypos;

    cam.handleMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cam.handleMouseScroll(yoffset);
}

void processInput(GLFWwindow *window, Camera &cam)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }

    // SETTINGS

    if(!tabPreviouslyPressed && glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (mouseLocked) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mouseLocked = false;
            firstMouse = true;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            mouseLocked = true;
        }
        
    }
    tabPreviouslyPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;

    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        cam.resetFov();
    }

    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        rotateLight = !rotateLight;
    }

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        cam.speed += 0.1f;
    }

    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        cam.speed -= 0.1f;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        coolLightMode = 0;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        coolLightMode = 1;
    }
    
    // MOVEMENT
    cam.deltaSpeed = cam.speed * deltaTime;
    
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.moveFront();
    }

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.moveBack();   
    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.moveLeft();
    }

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.moveRight();
    }

    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cam.moveWorldUp();
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cam.moveWorldDown();
    }
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
        return;
    }        
}

void setMaterial(Shader &shader, const material::Material &mat) {
    // shader.setVec3("material.diffuse", mat.diffuse);
    shader.setVec3("material.specular", mat.specular);
    shader.setFloat("material.shininess", mat.shininess);
}

#pragma endregion

void setLight(Shader &shader, const Light &light) {
    shader.setVec3("light.position", cam.position);

    // std::cout << "DEBUG << LIGHT DIRECTION: " << cam.front.x << " " << cam.front.y << " " << cam.front.z << "\n";
    shader.setVec3("light.direction", cam.front);

    shader.setFloat("light.cutOff",   glm::cos(glm::radians(12.5f)));
    shader.setFloat("light.outerCutOff",   glm::cos(glm::radians(17.5f)));


    shader.setVec3("light.ambient", light.ambient);
    shader.setVec3("light.diffuse", light.diffuse);
    shader.setVec3("light.specular", light.specular);

    shader.setFloat("light.constant", light.constant);
    shader.setFloat("light.linear", light.linear);
    shader.setFloat("light.quadratic", light.quadratic);
}