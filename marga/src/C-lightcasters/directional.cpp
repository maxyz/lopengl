#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <format>
#include <cmath>
#include "stb_image.h"
#include "shader.h"
#include "camera.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "texture_vertices.cpp"

const int INITIAL_WIDTH = 1024;
const int INITIAL_HEIGHT = 768;

float width = (float) INITIAL_WIDTH, height = (float) INITIAL_HEIGHT;

struct Light {
    float position[3];
    glm::vec3 color;
    float ambientStrength;
    float diffuseStrength;
    float specularStrength;
};

struct Material {
    float specular[3];
    float shininess;
};

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
    glViewport(0, 0, _width, _height);
    width = (float) _width;
    height = (float) _height;

}

// Mouse input requires a number of global variables :-/
Camera camera = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
double lastX = 400, lastY = 300;
bool firstMouse = true;

// Handle mouse input
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // Let ImGui handle the mouse
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;  // let ImGui handle it

    // Only move the camera when the cursor is captured
    if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
        return;

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

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Tab toggles capturing the mouse
    if ((key == GLFW_KEY_TAB) && (action == GLFW_RELEASE)) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetCursorPos(window, &lastX, &lastY);
            firstMouse = true;
        }
    }
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

}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "Lighting Maps", NULL, NULL);
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
    Shader ourShader("shaders/vertex.glsl", "shaders/fragment.glsl");
    Shader sourceShader("shaders/source-vertex.glsl", "shaders/source-frag.glsl");

    Texture::flip_vertically();
    Texture diffuseMap = Texture("../media/container2.png", GL_RGBA);
    Texture specularMap = Texture("../media/container2_specular_wood.png", GL_RGBA);

    ourShader.use();
    ourShader.setInt("material.diffuse", 0);
    ourShader.setInt("material.specular", 1);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // ..:: Create a light source object ::..
    // It has its own VAO.
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // we only need to bind to the VBO, the container's VBO's data already contains the data.
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // set the vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // We have 25 cubes
    glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3(-4.0f, -0.5f, -5.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -8.3f),
    glm::vec3( 2.4f, -3.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f),
    glm::vec3( 4.0f, -4.0f, -5.0f),
    glm::vec3(-4.5f, -2.2f, -2.5f),
    glm::vec3(-5.8f, -2.0f, -8.3f),
    glm::vec3( 6.4f, -0.4f, -3.5f),
    glm::vec3(-6.7f,  3.0f, -7.5f),
    glm::vec3( 6.3f, -2.0f, -2.5f),
    glm::vec3( 5.5f,  3.5f, -2.5f),
    glm::vec3( 4.5f,  0.2f, -1.5f),
    glm::vec3(-5.3f,  1.0f, -1.5f),
    glm::vec3(-2.2f, -3.5f, -2.5f),
    glm::vec3( 3.3f, -2.0f, -2.5f),
    glm::vec3( 4.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  4.2f, -1.5f),
    glm::vec3(-1.3f,  3.5f, -1.5f),
    glm::vec3(-3.3f,  4.0f, -4.5f)
    };

    float deltaTime = 0.0f;	// Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Capture the mouse and call the callback function
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, keyboard_callback);

    // Starting lighting values (position, color, ambient, diffuse, specular)
    Light light = { {1.2f, 1.0f, 2.0f}, glm::vec3( 1.0f,  1.0f,  1.0f), 0.2f, 0.5f, 1.0f, };
    // Starting material values
    Material material = { {0.5f, 0.5f, 0.5f}, 32 };

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.Fonts->AddFontFromFileTTF("../media/Roboto-Regular.ttf", 20);
    io.IniFilename = NULL; // Avoid creating imgui.ini

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

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

        // ImGui dock
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_Once);
        ImGui::Begin("Scene information", NULL, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::PushItemWidth(150.0f);
        ImGui::LabelText("Pos","(%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
        ImGui::LabelText("Front","(%.2f, %.2f, %.2f)", camera.Front.x, camera.Front.y, camera.Front.z);
        // Only show the controls when the mouse is not captured by the camera
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
            ImGui::SliderFloat("Shininess", &material.shininess, 0.0f, 256.0f);
            ImGui::ColorEdit3("Light Color", glm::value_ptr(light.color));
            ImGui::SliderFloat("Light Ambience", &light.ambientStrength, -1.0f, 1.0f);
            ImGui::SliderFloat("Light Diffuse", &light.diffuseStrength, -1.0f, 1.0f);
            ImGui::SliderFloat("Light Specular", &light.specularStrength, -1.0f, 1.0f);
        }
        ImGui::SeparatorText("");
        ImGui::Text("Press TAB to switch modes");
        ImGui::PopItemWidth();
        ImGui::End();

        // If we are not editing the colors, switch the light color with time
        //if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        //    light.color.x = sin(glfwGetTime() * 2.0f);
        //    light.color.y = sin(glfwGetTime() * 0.7f);
        //    light.color.z = sin(glfwGetTime() * 1.3f);
        //}

        glm::vec3 diffuseColor = light.color  * light.diffuseStrength;
        glm::vec3 ambientColor = diffuseColor * light.ambientStrength;
        glm::vec3 specularColor = light.color * light.specularStrength;

        ourShader.use();
        ourShader.setVec3f("viewPos", glm::value_ptr(camera.Position));

        ourShader.setVec3f("light.ambient", glm::value_ptr(ambientColor));
        ourShader.setVec3f("light.diffuse", glm::value_ptr(diffuseColor));
        ourShader.setVec3f("light.specular", glm::value_ptr(specularColor));

        // First cube material
        ourShader.setFloat("material.shininess", material.shininess);

        // Position of the light
        glm::vec3 lightPos(1.2f*sin(glfwGetTime()), 1.0f*cos(glfwGetTime()), 2.0f);
        ourShader.setVec3f("light.position", glm::value_ptr(lightPos));

        // Coordinate matrixes
        // ** View **
        glm::mat4 view = camera.GetViewMatrix();
        // ** Projection **
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.Zoom), width/height, 0.1f, 100.0f);

        ourShader.setMatrix4fv("view", glm::value_ptr(view));
        ourShader.setMatrix4fv("projection", glm::value_ptr(projection));

        // Bind the texture to draw
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap.ID);
    
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap.ID);

        // Draw each of the 10 cubes, with a different model matrix
        glBindVertexArray(VAO);
        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = (float)glfwGetTime() * 45.0f;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMatrix4fv("model", glm::value_ptr(model));
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
        sourceShader.setVec3f("lightColor", glm::value_ptr(light.color));
        // draw the light cube object
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // check and call events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }


    // Cleanup Imgui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Clean up
    glfwTerminate();
    return 0;
}

